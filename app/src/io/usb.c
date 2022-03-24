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
#define ABP_DEV_EUI_LEN   8
#define ABP_APP_EUI_LEN   8
#define ABP_APP_SKEY_LEN  16
#define ABP_NWK_SKEY_LEN  16

#define OTAA_APP_EUI  1
#define OTAA_JOIN_EUI 2
#define OTAA_APP_KEY  3
#define ABP_DEV_ADDR  4
#define ABP_DEV_EUI   5
#define ABP_APP_EUI   6
#define ABP_APP_SKEY  7
#define ABP_NWK_SKEY  8

extern struct lorawan_join_config join_cfg;

void shell_print_hex(const struct shell *shell, unsigned char *buf, int len)
{
    for (int i = 0; i < len; i++) {
        shell_print(shell, "%x%x", buf[i] / 16, buf[i] & 15);
    }
}

/* Converts a hex string to an unsigned char array */
unsigned char *hexstr_to_char(const char *hexstr, size_t len)
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

uint32_t uint8t_arr_to_uint32t(uint8_t *arr) {
    uint32_t out, temp;

    int i;
    printf("\ndevaddr:");
    for (i = 0; i < 4; i++)
        printf("%02x", arr[i]);
    printf("\n");

    out = (uint32_t) ((uint8_t)(arr[0]) << 24 |
                (uint8_t)(arr[1]) << 16 |
                (uint8_t)(arr[2]) << 8 |
                (uint8_t)(arr[3]));

    return out;
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

int config_lorawan_param(const struct shell *shell, unsigned char *str, 
                            int req_len, int param_type)
{
    int len;
    uint8_t *new_conf;
    uint8_t *prev_conf;
    uint32_t new_dev_addr;
    uint32_t prev_dev_addr;

    len = strlen(str);

    if (validate_input(shell, str, len, req_len) == -EINVAL) {
        return -EINVAL;
    }

    if (param_type == ABP_DEV_ADDR) {
        new_conf = hexstr_to_char(str, (size_t) len);
        new_dev_addr = uint8t_arr_to_uint32t(new_conf);
    } else {
        new_conf = hexstr_to_char(str, (size_t) len);
    }

    if (new_conf == NULL) {
        shell_error(shell, "Value must have an even number of hex digits.");
        return -EINVAL;
    }
        

    switch (param_type) {
        case OTAA_APP_EUI: 
            break;
        case OTAA_JOIN_EUI: 
            break;
        case OTAA_APP_KEY: 
            break;
        case ABP_DEV_ADDR:
            prev_dev_addr = join_cfg.abp.dev_addr;
            join_cfg.abp.dev_addr = new_dev_addr;
            break;
        case ABP_DEV_EUI:
            prev_conf = join_cfg.dev_eui;
            join_cfg.dev_eui = new_conf;
            break;
        case ABP_APP_EUI:
            prev_conf = join_cfg.abp.app_eui;
            join_cfg.abp.app_eui = new_conf;
            break;
        case ABP_APP_SKEY:
            prev_conf = join_cfg.abp.app_skey;
            join_cfg.abp.app_skey = new_conf;
            break;
        case ABP_NWK_SKEY:
            prev_conf = join_cfg.abp.nwk_skey;
            join_cfg.abp.nwk_skey = new_conf;
            break;
        default:
            break;
    }

    if (req_len == 8) {
        shell_print(shell, "prev: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                    prev_conf[0], prev_conf[1], prev_conf[2], prev_conf[3],
                    prev_conf[4], prev_conf[5], prev_conf[6], prev_conf[7]);

        shell_print(shell, "next: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                    new_conf[0], new_conf[1], new_conf[2], new_conf[3],
                    new_conf[4], new_conf[5], new_conf[6], new_conf[7]);
    } else if (req_len == 16) {
        shell_print(shell, "prev: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                    prev_conf[0], prev_conf[1], prev_conf[2], prev_conf[3],
                    prev_conf[4], prev_conf[5], prev_conf[6], prev_conf[7],
                    prev_conf[8], prev_conf[9], prev_conf[10], prev_conf[11],
                    prev_conf[12], prev_conf[13], prev_conf[14], prev_conf[15]);

        shell_print(shell, "next: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", 
                    new_conf[0], new_conf[1], new_conf[2], new_conf[3],
                    new_conf[4], new_conf[5], new_conf[6], new_conf[7],
                    new_conf[8], new_conf[9], new_conf[10], new_conf[11],
                    new_conf[12], new_conf[13], new_conf[14], new_conf[15]);

    } else if (req_len == 4) {
        
        shell_print(shell, "prev: %08x", prev_dev_addr);
        shell_print(shell, "next: %08x", new_dev_addr);
    }

    free(new_conf);

    return 0;
}

/* OTAA Handlers */
static int otaa_app_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_APP_EUI_LEN, OTAA_APP_EUI);
}

static int otaa_join_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_JOIN_EUI_LEN, OTAA_JOIN_EUI);
}

static int otaa_app_key(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], OTAA_APP_KEY_LEN, OTAA_APP_KEY);
}

/* ABP Handlers */
static int abp_dev_addr(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_DEV_ADDR_LEN, ABP_DEV_ADDR);
}

static int abp_dev_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_DEV_EUI_LEN, ABP_DEV_EUI);
}

static int abp_app_eui(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_APP_EUI_LEN, ABP_APP_EUI);
}

static int abp_app_skey(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_APP_SKEY_LEN, ABP_APP_SKEY);
}

static int abp_nwk_skey(const struct shell *shell, size_t argc, char **argv)
{
    return config_lorawan_param(shell, argv[1], ABP_NWK_SKEY_LEN, ABP_NWK_SKEY);
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
