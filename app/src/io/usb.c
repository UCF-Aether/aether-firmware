#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <shell/shell.h>
#include <lorawan/lorawan.h>

/* Parameters lengths in bytes */
#define OTAA_APP_EUI_LEN  8
#define OTAA_JOIN_EUI_LEN 8
#define OTAA_APP_KEY_LEN  16

#define ABP_DEV_ADDR_LEN  4
#define ABP_JOIN_EUI_LEN  8
#define ABP_DEV_EUI_LEN   8
#define ABP_APP_EUI_LEN   8
#define ABP_APP_SKEY_LEN  16
#define ABP_NWK_SKEY_LEN  16

extern struct lorawan_join_config join_cfg;

void shell_print_hex(const struct shell *shell, unsigned char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        shell_print(shell, "%x%x", buf[i] / 16, buf[i] & 15);
    }
}

/* Converts a hex string to an unsigned char array */
unsigned char *hexstr_to_char(const char* hexstr, size_t len)
{
    if (len % 2 != 0)
        return NULL;

    size_t final_len = len / 2;
    unsigned char *chrs = (unsigned char *) malloc((final_len+1) * sizeof(*chrs));

    for (size_t i = 0, j = 0; j < final_len; i += 2, j++)
        chrs[j] = (hexstr[i] % 32 + 9) % 25 * 16 + (hexstr[i+1] % 32 + 9) % 25;

    chrs[final_len] = '\0';

    return chrs;
}

/* Determines if input string meets requirement for given LoRaWAN parameter */
int validate_input(const struct shell *shell, unsigned char *buf, int len, int req_len)
{
    int i;

    if (len % 2 != 0) {
        shell_error(shell, "String must be an even length.");
        return -EINVAL;
    }

    if (len/2 != req_len) {
        shell_error(shell, "DevEUI must be %d bytes long. Provided string is %d bytes.", req_len, len/2);
        return -EINVAL;
    }

    for (i = 0; i < len; i++) {
        if (!isxdigit(buf[i])) {
            shell_error(shell, "Value can only contain digits 0-9 and a-f.");
            return -EINVAL;
        }
    }
            
    return 1;
}

int config_lorawan_param(const struct shell *shell, unsigned char *str, int req_len)
{
    int len;
    uint8_t *new_conf;

    len = strlen(str);

    if (validate_input(shell, str, len, req_len) == -EINVAL) {
        return -EINVAL;
    }

    new_conf = hexstr_to_char(str, (size_t) len);

    if (new_conf == NULL) {
        shell_error(shell, "Value must have an even number of hex digits.");
        return -EINVAL;
    }
        
    shell_print(shell, "prev: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                join_cfg.dev_eui[0], join_cfg.dev_eui[1],
                join_cfg.dev_eui[2], join_cfg.dev_eui[3],
                join_cfg.dev_eui[4], join_cfg.dev_eui[5],
                join_cfg.dev_eui[6], join_cfg.dev_eui[7]);

    shell_print(shell, "next: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                new_conf[0], new_conf[1],
                new_conf[2], new_conf[3],
                new_conf[4], new_conf[5],
                new_conf[6], new_conf[7]);

    free(new_conf);
}

/* OTAA Handlers */
static int otaa_app_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_APP_EUI_LEN);
}

static int otaa_join_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_JOIN_EUI_LEN);
}

static int otaa_app_key(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_APP_KEY_LEN);
}

/* ABP Handlers */
static int abp_dev_addr(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_DEV_ADDR_LEN);
}

static int abp_dev_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_DEV_EUI_LEN);
}

static int abp_app_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_APP_EUI_LEN);
}

static int abp_app_skey(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_APP_SKEY_LEN);
}

static int abp_nwk_skey(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_NWK_SKEY_LEN);
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
