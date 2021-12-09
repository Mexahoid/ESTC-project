#include "usb_commands.h"
#include "usb_commands_color_utils.h"

// Delegate that gets current RGB from RGB module.
static void (*get_rgb)(color_rgb_t*);
// Special struct that holds USB data.
usb_data_t* usb_data;
// Message between submodules size.
#define USBC_BUFF_MESSAGE_SIZE 1024

// Max symbols in name (not counting \0).
#define MAX_NAME_LEN 8
// Max colors that can be saved.
#define MAX_COLORS 10

// Holds info about RGB.
typedef struct
{
    uint8_t r;
    uint8_t g;
    uint8_t b;
} cmd_args_rgb_t;

// Holds info about HSV.
typedef struct
{
    uint16_t h;
    uint8_t  s;
    uint8_t  v;
} cmd_args_hsv_t;

// Holds info about color name and values.
typedef struct
{
    bool saved;
    union
    {
        cmd_args_hsv_t hsv;
        cmd_args_rgb_t rgb;
    } color;
    char name[MAX_NAME_LEN + 1];
} cmd_args_add_color_t;

// Holds command and its handler.
typedef struct
{
    usb_command_t command;
    void (*handler)(char*, char*);
    const char* info;
} cmd_handler_t;

// Handles CURR command.
static void handler_get(char* text_buff, char *args);
// Handles RGB command.
static void handler_rgb(char* text_buff, char *args);
// Handles HSV command.
static void handler_hsv(char* text_buff, char *args);
// Handles HELP command.
static void handler_help(char* text_buff, char *args);
// Handles LIST command.
static void handler_list(char* text_buff, char *args);
// Handles ADDRGB command.
static void handler_add_rgb(char* text_buff, char *args);
// Handles ADDCURR command.
static void handler_add_curr(char* text_buff, char *args);
// Handles DEL command.
static void handler_del(char* text_buff, char *args);
// Handles SET command.
static void handler_set(char* text_buff, char *args);
// Handles wrong commands.
static void handler_unknown(char* text_buff, char *args);

// Commands with their handlers and info.
static cmd_handler_t cmdhdls[] =
{
    { USB_COM_GET_RGB,  &handler_get,       "CURR\r\n- Gets current RGB color.\r\n"},
    { USB_COM_RGB,      &handler_rgb,       "RGB <r> <g> <b>\r\n- Changes RGB. Color codes should be between 0 and 255.\r\n"},
    { USB_COM_HSV,      &handler_hsv,       "HSV <h> <s> <v>\r\n- Changes HSV. Color code h should be between 0 and 360, s and v - between 0 and 100.\r\n"},
    { USB_COM_HELP,     &handler_help,      "HELP\r\n- You are here.\r\n"},
    { USB_COM_LIST,     &handler_list,      "LIST \r\n- Lists saved color names.\r\n"},
    { USB_COM_ADD_RGB,  &handler_add_rgb,   "ADDRGB <r> <g> <b> <name>\r\n- Adds RGB color under a name.\r\n"},
    { USB_COM_ADD_CURR, &handler_add_curr,  "ADDCURR <name>\r\n- Adds current color under a name.\r\n"},
    { USB_COM_DEL,      &handler_del,       "DEL <name>\r\n- Deletes named color.\r\n"},
    { USB_COM_SET,      &handler_set,       "SET <name>\r\n- Sets named color as current.\r\n"},
    { USB_COM_UNKNOWN,  &handler_unknown,   "Any other command will result in Unknown command message.\r\n"}
};

// Colors array.
cmd_args_add_color_t colors_names[MAX_COLORS];

// Strtoks string to an array of strings.
static bool get_args(char *word, char **arr, int count)
{
    int i = 0;
    char *pot_nums = strtok(word, " ");
    // To remove first word
    pot_nums = strtok(NULL, " ");

    while (pot_nums != NULL)
    {
        if (i >= count)
            return false;
        arr[i++] = pot_nums;
        pot_nums = strtok(NULL, " ");
    }
    return i == count;
}

static void handler_rgb(char* text_buff, char *args)
{
    NRF_LOG_INFO("[USB RX] Set RGB");
    int count = 3;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong color sequence.\r\n");
        return;
    }


    int nums[count];
    for (int i = 0; i < count; i++)
    {
        if (sscanf(arr[i], "%d", &nums[i]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Wrong data: %s", arr[i]);
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong data passed: %s.\r\n", arr[i]);
            return;
        }
    }
    if (!helper_is_inbetween(nums[0], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Red invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Red color code is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[1], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Green invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Green color code is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[2], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Blue invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Blue color code is invalid.\r\n");
        return;
    }
    NRF_LOG_INFO("[USB RX] RGB color set to R: %d, G: %d, B: %d.", nums[0], nums[1], nums[2]);
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> RGB color set to R: %d, G: %d, B: %d.\r\n", nums[0], nums[1], nums[2]);

    usb_data->field1 = nums[0];
    usb_data->field2 = nums[1];
    usb_data->field3 = nums[2];
    usb_data->usb_color_command = USB_COM_RGB;

}

static void handler_hsv(char* text_buff, char *args)
{
    NRF_LOG_INFO("[USB RX] Set HSV");
    int count = 3;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong color sequence.\r\n");
        return;
    }


    int nums[count];
    for (int i = 0; i < count; i++)
    {
        if (sscanf(arr[i], "%d", &nums[i]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Wrong data: %s", arr[i]);
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong data passed: %s.\r\n", arr[i]);
            return;
        }
    }
    if (!helper_is_inbetween(nums[0], 0, 360))
    {
        NRF_LOG_INFO("[USB RX] Hue invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Hue is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[0], 0, 100))
    {
        NRF_LOG_INFO("[USB RX] Sat invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Saturation is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[0], 0, 100))
    {
        NRF_LOG_INFO("[USB RX] Bri invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Brightness is invalid.\r\n");
        return;
    }

    NRF_LOG_INFO("[USB RX] HSV color set to H: %d, S: %d, V: %d.", nums[0], nums[1], nums[2]);
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> HSV color set to H: %d, S: %d, V: %d.\r\n", nums[0], nums[1], nums[2]);

    usb_data->field1 = nums[0];
    usb_data->field2 = nums[1];
    usb_data->field3 = nums[2];
    usb_data->usb_color_command = USB_COM_HSV;
}

static void handler_get(char* text_buff, char *args)
{
    UNUSED_VARIABLE(args);
    color_rgb_t clr;
    get_rgb(&clr);
    NRF_LOG_INFO("[USB RX] Requested RGB, current: R: %ld, G: %ld, B: %ld.", clr.r, clr.g, clr.b);
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Current RGB color: R: %ld, G: %ld, B: %ld.\r\n", clr.r, clr.g, clr.b);
}

static void handler_unknown(char* text_buff, char *args)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] Unknown command.");
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Unknown command. Type \"HELP\" to list available commands.\r\n");
}

static void handler_help(char* text_buff, char *args)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] Asking for help.");
    char info[USBC_BUFF_MESSAGE_SIZE];
    memset(info, 0, USBC_BUFF_MESSAGE_SIZE);

    strcat(info, "\r\n== Commands ==\r\n");
    for (int i = 0; i < sizeof(cmdhdls)/sizeof(cmdhdls[0]); i++)
    {
        strcat(info, cmdhdls[i].info);
    }
    strcat(info, "== End ==\r\n");

    strcpy(text_buff, info);
}

static void handler_list(char* text_buff, char *args)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] List requested.");
    if (!colors_names[0].saved)
    {
        NRF_LOG_INFO("[USB RX] No saved colors.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> No saved colors.\r\n");
        return;
    }

    NRF_LOG_INFO("[USB RX] Saved colors:");
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Saved colors:\r\n");

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            return;

        char info[64];
        memset(info, 0, 64);
        snprintf(info, 64, "%d. %s, R: %d, G: %d, B: %d\r\n", (i + 1), colors_names[i].name, colors_names[i].color.rgb.r,
                                                                                        colors_names[i].color.rgb.g,
                                                                                        colors_names[i].color.rgb.b);
        NRF_LOG_INFO("[USB RX] %s", info);
        strcat(text_buff, info);
    }
}

static void handler_add_rgb(char* text_buff, char *args)
{
    NRF_LOG_INFO("[USB RX] Adding RGB record.");
    if (colors_names[MAX_COLORS - 1].saved)
    {
        NRF_LOG_INFO("[USB RX] Max colors present.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Max colors saved, can't add new.\r\n");
        return;
    }

    int count = 4;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong ADDRGB command syntax.\r\n");
        return;
    }

    if (strlen(arr[3]) > MAX_NAME_LEN)
    {
        NRF_LOG_INFO("[USB RX] Name is too long: %s", arr[3]);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> The name is too long, can't exceed %d.\r\n", MAX_NAME_LEN);
        return;
    }

    int nums[count - 1];
    for (int i = 0; i < count - 1; i++)
    {
        if (sscanf(arr[i], "%d", &nums[i]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Wrong data: %s", arr[i]);
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong data passed: %s.\r\n", arr[i]);
            return;
        }
    }

    if (!helper_is_inbetween(nums[0], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Red invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Red color code is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[1], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Green invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Green color code is invalid.\r\n");
        return;
    }
    if (!helper_is_inbetween(nums[2], 0, 255))
    {
        NRF_LOG_INFO("[USB RX] Blue invalid.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Blue color code is invalid.\r\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (strcmp(colors_names[i].name, arr[3]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Name is already present.");
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> That name is already present.\r\n");
            return;
        }

        if (colors_names[i].saved)
            continue;

        strcpy(colors_names[i].name, arr[3]);
        cmd_args_rgb_t rgb;
        rgb.r = nums[0];
        rgb.g = nums[1];
        rgb.b = nums[2];
        colors_names[i].color.rgb = rgb;
        colors_names[i].saved = true;
        NRF_LOG_INFO("[USB RX] Savind current color.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Successfully saved new color. Colors saved: %d.\r\n", (i + 1));
        return;
    }
}

static void handler_add_curr(char* text_buff, char *args)
{
    color_rgb_t clr;
    get_rgb(&clr);
    NRF_LOG_INFO("[USB RX] Adding current color record, current: R: %d, G: %d, B: %d.", clr.r, clr.g, clr.b);
    if (colors_names[MAX_COLORS - 1].saved)
    {
        NRF_LOG_INFO("[USB RX] Max colors present.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Max colors saved, can't save current.\r\n");
        return;
    }

    int count = 1;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong ADDCURR command syntax.\r\n");
        return;
    }

    if (strlen(arr[0]) > MAX_NAME_LEN)
    {
        NRF_LOG_INFO("[USB RX] Name is too long: %s", arr[0]);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> The name is too long, can't exceed %d.\r\n", MAX_NAME_LEN);
        return;
    }


    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (strcmp(colors_names[i].name, arr[0]) == 0)
        {
            NRF_LOG_INFO("[USB RX] Name is already present.");
            snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> That name is already present.\r\n");
            return;
        }

        if (colors_names[i].saved)
            continue;


        color_rgb_t clr;
        get_rgb(&clr);

        strcpy(colors_names[i].name, arr[0]);
        cmd_args_rgb_t rgb;
        rgb.r = clr.r;
        rgb.g = clr.g;
        rgb.b = clr.b;
        colors_names[i].color.rgb = rgb;
        colors_names[i].saved = true;
        NRF_LOG_INFO("[USB RX] Saved current color.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Successfully saved current color. Colors saved: %d.\r\n", (i + 1));
        return;
    }
}

static void handler_del(char* text_buff, char *args)
{
    NRF_LOG_INFO("[USB RX] Deleting record.");
    int count = 1;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong DEL command syntax.\r\n");
        return;
    }

    if (strlen(arr[0]) > MAX_NAME_LEN)
    {
        NRF_LOG_INFO("[USB RX] Name is too long: %s", arr[0]);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> The name is too long, there can't be such name.\r\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            break;

        if (strcmp(colors_names[i].name, arr[0]) != 0)
            continue;

        for (int j = i + 1; j < MAX_COLORS; j++)
        {
            strcpy(colors_names[j - 1].name, colors_names[j].name);
            colors_names[j - 1].color = colors_names[j].color;
            colors_names[j - 1].saved = colors_names[j].saved;
        }

        NRF_LOG_INFO("[USB RX] Removed color.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Removed saved color %s.\r\n", arr[0]);
        return;

    }

    NRF_LOG_INFO("[USB RX] No name found.");
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Specified name not found.\r\n");
}

static void handler_set(char* text_buff, char *args)
{
    NRF_LOG_INFO("[USB RX] Setting color by name.");
    int count = 1;
    char *arr[count];
    if (!get_args(args, arr, count))
    {
        NRF_LOG_INFO("[USB RX] Wrong syntax: %s", args);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Wrong SET command syntax.\r\n");
        return;
    }

    if (strlen(arr[0]) > MAX_NAME_LEN)
    {
        NRF_LOG_INFO("[USB RX] Name is too long: %s", arr[0]);
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> The name is too long, there can't be such name.\r\n");
        return;
    }

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            break;

        if (strcmp(colors_names[i].name, arr[0]) != 0)
            continue;

        usb_data->field1 = colors_names[i].color.rgb.r;
        usb_data->field2 = colors_names[i].color.rgb.g;
        usb_data->field3 = colors_names[i].color.rgb.b;
        usb_data->usb_color_command = USB_COM_RGB;

        NRF_LOG_INFO("[USB RX] Set color.");
        snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n>> Set current color: %s.\r\n", arr[0]);
        return;

    }

    NRF_LOG_INFO("[USB RX] No such name.");
    snprintf(text_buff, USBC_BUFF_MESSAGE_SIZE, "\r\n> Specified name not found.\r\n");
}

// Parses prefix from command.
static usb_command_t parse_prefix(char *text)
{
    if (strncmp(text, "CURR", 4) == 0)
        return USB_COM_GET_RGB;
    if (strncmp(text, "RGB", 3) == 0)
        return USB_COM_RGB;
    if (strncmp(text, "HSV", 3) == 0)
        return USB_COM_HSV;
    if (strncmp(text, "HELP", 4) == 0)
        return USB_COM_HELP;
    if (strncmp(text, "ADDRGB", 6) == 0)
        return USB_COM_ADD_RGB;
    if (strncmp(text, "ADDCURR", 7) == 0)
        return USB_COM_ADD_CURR;
    if (strncmp(text, "DEL", 3) == 0)
        return USB_COM_DEL;
    if (strncmp(text, "SET", 3) == 0)
        return USB_COM_SET;
    if (strncmp(text, "LIST", 4) == 0)
        return USB_COM_LIST;

    return USB_COM_UNKNOWN;
}

void usbc_process_command(char *buff, char *command_buff)
{
    usb_command_t command = parse_prefix(command_buff);

    for (int i = 0; i < sizeof(cmdhdls)/sizeof(cmdhdls[0]); i++)
    {
        if (cmdhdls[i].command == command)
        {
            cmdhdls[i].handler(buff, command_buff);
            return;
        }
    }

}

void usbc_init(usb_data_t *usbd, void (*action)(color_rgb_t*))
{
    get_rgb = action;
    usb_data = usbd;
}