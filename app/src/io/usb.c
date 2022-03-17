#include <shell/shell.h>

#define OTAA_APP_EUI  1
#define OTAA_JOIN_EUI 2
#define OTAA_APP_KEY  3

#define ABP_DEV_ADDR  1
#define ABP_JOIN_EUI  2
#define ABP_DEV_EUI   3
#define ABP_APP_EUI   4
#define ABP_APP_SKEY  5
#define ABP_NWK_SKEY  6


static int otaa_app_eui(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "OTAA AppEUI");

    return 0;
}

static int otaa_join_eui(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "OTAA JoinEUI");

    return 0;
}

static int otaa_app_key(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "OTAA AppKey");

    return 0;
}

/* ABP Handlers */

static int abp_dev_addr(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP DevAddr");

    return 0;
}

static int abp_join_eui(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP JoinEUI");

    return 0;
}

static int abp_dev_eui(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP DevEUI");

    return 0;
}

static int abp_app_eui(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP AppEUI");

    return 0;
}

static int abp_app_skey(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP AppSKey");

    return 0;
}

static int abp_nwk_skey(const struct shell *shell, size_t argc, 
                                char **argv, void *data)
{
    int param;

    param = (int) data;

    shell_print(shell, "argc: %d", argc);
    shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);
    shell_print(shell, "ABP NwkSKey");

    return 0;
}


/* OTAA sub commands */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_otaa,
    SHELL_CMD_ARG(app_eui,  NULL, "Configure AppEUI.",  otaa_app_eui,  2, 0),
    SHELL_CMD_ARG(join_eui, NULL, "Configure JoinEUI.", otaa_join_eui, 2, 0),
    SHELL_CMD_ARG(app_key,  NULL, "Configure AppKey.",  otaa_app_key,  2, 0),
    SHELL_SUBCMD_SET_END
);

/* ABP sub commands */
SHELL_STATIC_SUBCMD_SET_CREATE(sub_abp,
    SHELL_CMD_ARG(dev_addr, NULL, "Configure DevAddr.", abp_dev_addr, 2, 0),
    SHELL_CMD_ARG(join_eui, NULL, "Configure JoinEUI.", abp_join_eui, 2, 0),
    SHELL_CMD_ARG(dev_eui,  NULL, "Configure DevEUI.",  abp_dev_eui,  2, 0),
    SHELL_CMD_ARG(app_eui,  NULL, "Configure AppEUI.",  abp_app_eui,  2, 0),
    SHELL_CMD_ARG(app_skey, NULL, "Configure AppSKey.", abp_app_skey, 2, 0),
    SHELL_CMD_ARG(nwk_skey, NULL, "Configure NwkSkey.", abp_nwk_skey, 2, 0),
    SHELL_SUBCMD_SET_END
);

SHELL_STATIC_SUBCMD_SET_CREATE(sub_lorawan_config,
    SHELL_CMD(abp,   &sub_abp, "Configure ABP parameters.", NULL),
    SHELL_CMD(otaa,   &sub_otaa, "Configure OTAA parameters.", NULL),
    SHELL_SUBCMD_SET_END
);

SHELL_CMD_REGISTER(lorawan_config, &sub_lorawan_config, 
                   "Configure the LoRaWAN parameters", NULL);

// /* OTAA handlers */
// static int cmd_otaa_handler(const struct shell *shell, size_t argc, 
//                                 char **argv, void *data)
// {
//     // ARG_UNUSED(argc);
//     // ARG_UNUSED(argv);

//     int param;

//     param = (int) data;

//     switch(param) {
//         case OTAA_APP_EUI:
//             shell_print(shell, "configure otaa app eui");
//             break;
//         case OTAA_JOIN_EUI:
//             shell_print(shell, "configure otaa join eui");
//             break;
//         case OTAA_APP_KEY:
//             shell_print(shell, "configure otaa app key");
//             break;
//     }
//         return 0;
// }



// /* ABP handlers */
// static int cmd_abp_handler(const struct shell *shell, size_t argc, 
//                                 char **argv, void *data)
// {
//     // ARG_UNUSED(argc);
//     // ARG_UNUSED(argv);

//     int param;

//     param = (int) data;

//     shell_print(shell, "argc: %d", argc);
//     shell_print(shell, "arg0: %s arg1: %s", argv[0], argv[1]);

//     switch(param) {
//         case ABP_DEV_ADDR:
//             shell_print(shell, "configure abp dev addr");
//             break;
//         case ABP_JOIN_EUI:
//             shell_print(shell, "configure abp join eui");
//             break;
//         case ABP_DEV_EUI:
//             shell_print(shell, "configure abp dev eui");
//             break;
//         case ABP_APP_EUI:
//             shell_print(shell, "configure abp app eui");
//             break;
//         case ABP_APP_SKEY:
//             shell_print(shell, "configure abp app skey");
//             break;
//         case ABP_NWK_SKEY:
//             shell_print(shell, "configure abp nwk skey");
//             break;
//     }

//     return 0;
// }

// /* Confiugre OTAA values */
// SHELL_SUBCMD_DICT_SET_CREATE(sub_otaa, cmd_otaa_handler,
//     (app_eui, OTAA_APP_EUI), 
//     (join_eui, OTAA_JOIN_EUI), 
//     (app_key, OTAA_JOIN_EUI)
// );

// /* Configure ABP values */
// SHELL_SUBCMD_DICT_SET_CREATE(sub_abp, cmd_abp_handler,
//     (dev_addr, ABP_DEV_ADDR), 
//     (join_eui, ABP_JOIN_EUI), 
//     (dev_eui,  ABP_DEV_EUI),
//     (app_eui,  ABP_APP_EUI),
//     (app_skey, ABP_APP_SKEY),
//     (nwk_skey, ABP_NWK_SKEY)
// );

// SHELL_STATIC_SUBCMD_SET_CREATE(sub_lorawan_config,
//     SHELL_CMD_ARG(abp,   &sub_abp, "Configure ABP parameters.", NULL, 2, 2),
//     SHELL_CMD_ARG(otaa,   &sub_otaa, "Configure OTAA parameters.", NULL, 2, 2),
//     SHELL_SUBCMD_SET_END
// );

// SHELL_CMD_REGISTER(lorawan_config, &sub_lorawan_config, 
//                    "Configure the LoRaWAN parameters", NULL);