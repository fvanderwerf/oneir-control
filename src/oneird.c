
#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "cge.h"
#include "oneir_app.h"
#include "hex.h"

#include <json.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <argp.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

volatile int quit = 0;

struct config {
    const char *unixsock;
    int daemonize;
} config = { 
    NULL,
    0
};

struct oneir_app_config app_config = {
    .gpio_sclk = 11,
    .gpio_miso = 9,
    .gpio_mosi = 10,
    .gpio_reset = 25,
    .gpio_smbdat = 2,
    .gpio_smbclk = 3
};


void signal_handler(int signal)
{
    quit = 1;
}

struct argp_option options[] = {
    {
        .name = "daemonize",
        .key = 'd',
    },
    { 0 }
};

error_t parse_argument(int key, char *arg, struct argp_state *state)
{
    switch(key) {
        case ARGP_KEY_ARG:
            if (config.unixsock == NULL)
                config.unixsock = arg;
            else
                return ARGP_ERR_UNKNOWN;
            break;
        case 'd':
            config.daemonize = 1;
            break;
        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


void parse_arguments(int argc, char *argv[])
{
    struct argp argp;

    argp_program_version = PACKAGE_VERSION;
    argp_program_bug_address = PACKAGE_BUGREPORT;
    argp_program_version_hook = NULL;
    argp_err_exit_status = 0;

    argp.options = options;
    argp.parser = parse_argument;
    argp.args_doc = "<unix_domain_socket>";
    argp.doc = NULL;
    argp.children = NULL;
    argp.help_filter = NULL;
    argp.argp_domain = NULL;

    argp_parse(&argp, argc, argv, 0, NULL, NULL);

}

static int handle_json_rc5(struct json_object *object, struct oneir_app *app)
{
    struct json_object *address, *code;

    CGE(json_object_get_type(object) != json_type_object);

    CGE(!json_object_object_get_ex(object, "address", &address));
    CGE(!json_object_object_get_ex(object, "code", &code));

    CGE(json_object_get_type(address) != json_type_int);
    CGE(json_object_get_type(code) != json_type_int);

    CGE_NEG(oneir_mcu_send_rc5(app->mcu,
            json_object_get_int(address),
            json_object_get_int(code)));

    return 0;
error:
    return -1;
}

int handle_json_raw(struct json_object *object, struct oneir_app *app)
{
    const char *hex_command = NULL;
    int hex_command_len;
    char *command = NULL;

    CGE(json_object_get_type(object) != json_type_string);

    hex_command_len = json_object_get_string_len(object);

    CGE_NULL(command = malloc(hex_command_len/2));

    hex_command = json_object_get_string(object);

    CGE_NEG(hex_decode(command, hex_command, hex_command_len));

//    CGE_NEG(oneir_mcu_send_raw(app->mcu,
                //json_object

    return 0;
error:
    if (command)
        free(command);

    return -1;
}


int handle_json(struct json_object *object, struct oneir_app *app)
{
    struct json_object *type, *command;
    const char *strtype;

    CGE(!json_object_object_get_ex(object, "type", &type));
    CGE(!json_object_object_get_ex(object, "command", &command));

    CGE(json_object_get_type(type) != json_type_string);

    strtype = json_object_get_string(type);

    if (strcmp(strtype, "rc5") == 0) {
        CGE_NEG(handle_json_rc5(command, app));
    } else if (strcmp(strtype, "raw") == 0) {
        CGE_NEG(handle_json_raw(command, app));
    } else {
        GE();
    }

    return 0;
error:
    return -1;
}


void handle_client(int client, struct oneir_app *app)
{
    struct json_tokener *json = NULL;
    struct json_object *object = NULL;
    
    ssize_t numbytes;
    char buf[100];
    const char *okreply = "{ 'status' : 'ok' }";
    const char *nokreply = "{ 'status' : 'nok' }";

    CGE_NULL(json = json_tokener_new());

    while (1) {
        numbytes = recv(client, buf, sizeof(buf), 0);

        if (numbytes > 0) {

            object = json_tokener_parse_ex(json, buf, numbytes);

            if (object != NULL) {
                break;
            } else {
                CGE(json_tokener_get_error(json) != json_tokener_continue);
            }
        }
    }

    CGE_NEG(handle_json(object, app));

    json_object_put(object);
    object == NULL;

    send(client, okreply, strlen(okreply), 0);
    shutdown(client, SHUT_RDWR);

    return;
error:

    if (object != NULL)
        json_object_put(object);

    if (json != NULL)
        json_tokener_free(json);

    send(client, nokreply, strlen(nokreply), 0);
    shutdown(client, SHUT_RDWR);
}

int create_unix_server_socket(const char *path)
{
    int s = -1;
    struct sockaddr_un addr;

    if (access(path, F_OK) == 0) {
        unlink(path);
    }

    CGE_NEG(s = socket(AF_UNIX, SOCK_STREAM, 0));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, config.unixsock);
    CGE_NEG(bind(s, (struct sockaddr *) &addr, sizeof(addr)));

    CGE_NEG(listen(s, 5));

    return s;
error:

    if (s >= 0) {
        close(s);
    }
    return -1;
}

void check_config()
{
    if (config.unixsock == NULL) {
        fprintf(stderr, "Define a unix domain socket to listen to\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[])
{
    int unixdomain = -1;
    struct sockaddr_un addr;
    struct oneir_app *oneir;

    parse_arguments(argc, argv);

    check_config();

    CGE_NULL(oneir = construct_app(&app_config));

    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    CGE_NEG(unixdomain = create_unix_server_socket(config.unixsock));

    if (config.daemonize) {
        CGE_NEG(daemon(0, 0));
    }

    while (!quit) {
        int client = accept(unixdomain, NULL, NULL);

        if (client >= 0)
            handle_client(0, /*client*/ oneir);
    }

    close(unixdomain);
    unlink(config.unixsock);

    exit(EXIT_SUCCESS);

error:
    if (oneir != NULL)
        destroy_app(oneir);

    if (unixdomain >= 0) {
        close(unixdomain);
    }

    exit(EXIT_FAILURE);
}

