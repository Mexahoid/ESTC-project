#include "usb_commands.h"
#include "usb_commands_color_utils.h"

// Delegate that gets current RGB from RGB module.
static void (*get_rgb)(color_rgb_t*);
// Special struct that holds USB data.
static usb_data_t* usb_data;

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

// Holds color values.
typedef union
{
    cmd_args_hsv_t hsv;
    cmd_args_rgb_t rgb;
} cmd_args_inner_color_union_t;

// Holds info about color name and values.
typedef struct
{
    bool saved;
    usb_color_t color_type;
    cmd_args_inner_color_union_t color;
    char name[MAX_NAME_LEN + 1];
} cmd_args_add_color_t;

// Holds command and its handler.
typedef struct
{
    const char* command;
    void (*handler)(char*, const char*, int);
    const char* args;
    const char* info;
} cmd_handler_t;

/**
 * @brief Fills usb_data with data
 *
 * @param values                Color union with data
 * @param color                 Color type
 */
static void fill_usb_data(const cmd_args_inner_color_union_t * const values, usb_color_t color)
{
    usb_data->usb_color = color;
    switch (color)
    {
        case USB_COLOR_RGB:
            usb_data->field1 = values->rgb.r;
            usb_data->field2 = values->rgb.g;
            usb_data->field3 = values->rgb.b;
            break;
        case USB_COLOR_HSV:
            usb_data->field1 = values->hsv.h;
            usb_data->field2 = values->hsv.s;
            usb_data->field3 = values->hsv.v;
            break;
    }
}


// Handles CURR command.
static void handler_get(char* const text_buff, const char * const args, int buff_msg_size);
// Handles RGB command.
static void handler_rgb(char* const text_buff, const char * const args, int buff_msg_size);
// Handles HSV command.
static void handler_hsv(char* const text_buff, const char * const args, int buff_msg_size);
// Handles HELP command.
static void handler_help(char* const text_buff, const char * const args, int buff_msg_size);
// Handles LIST command.
static void handler_list(char* const text_buff, const char * const args, int buff_msg_size);
// Handles ADDRGB command.
static void handler_add_rgb(char* const text_buff, const char * const args, int buff_msg_size);
// Handles ADDHSV command.
static void handler_add_hsv(char* const text_buff, const char * const args, int buff_msg_size);
// Handles ADDCURR command.
static void handler_add_curr(char* const text_buff, const char * const args, int buff_msg_size);
// Handles DEL command.
static void handler_del(char* const text_buff, const char * const args, int buff_msg_size);
// Handles SET command.
static void handler_set(char* const text_buff, const char * const args, int buff_msg_size);
// Handles wrong commands.
static void handler_unknown(char* const text_buff, const char * const args, int buff_msg_size);

// Commands with their handlers and info.
static cmd_handler_t cmdhdls[] =
{
    { "CURR",       &handler_get,       "",                     "- Gets current RGB color."},
    { "RGB",        &handler_rgb,       "<r> <g> <b>",          "- Changes RGB. Color codes should be between 0 and 255."},
    { "HSV",        &handler_hsv,       "<h> <s> <v>",          "- Changes HSV. Color code h should be between 0 and 360, s and v - between 0 and 100."},
    { "HELP",       &handler_help,      "",                     "- You are here."},
    { "LIST",       &handler_list,      "",                     "- Lists saved color names."},
    { "ADDRGB",     &handler_add_rgb,   "<r> <g> <b> <name>",   "- Adds RGB color under a name."},
    { "ADDHSV",     &handler_add_hsv,   "<h> <s> <v> <name>",   "- Adds HSV color under a name."},
    { "ADDCURR",    &handler_add_curr,  "<name>",               "- Adds current color as RGB under a name."},
    { "DEL",        &handler_del,       "<name>",               "- Deletes named color."},
    { "SET",        &handler_set,       "<name>",               "- Sets named color as current."},
};

// Colors array.
static cmd_args_add_color_t colors_names[MAX_COLORS];

/**
 * @brief Saves data in color record.
 *
 * @param colors_name               Pointer to a color record
 * @param data                      Data for saving
 * @param color                     Color type
 */
static void save_color_data(cmd_args_add_color_t * const colors_name, const cmd_args_inner_color_union_t * const data, usb_color_t color)
{
    colors_name->color_type = color;
    colors_name->saved = true;
    switch (color)
    {
    case USB_COLOR_RGB:
        colors_name->color.rgb = data->rgb;
        break;
    case USB_COLOR_HSV:
        colors_name->color.hsv = data->hsv;
        break;

    default:
        break;
    }
}

/**
 * @brief Parses USBCCU code and prints message, return true if it's USBCCU_OK.
 *
 * @param text_buff                 Buffer for messages
 * @param buff_msg_size             Buffer length
 * @param code                      USBCCU code
 * @return true
 * @return false
 */
static bool parse_usbccu_code(char* const text_buff, int buff_msg_size, usbccu_error_t code)
{
    switch (code)
    {
    case USBCCU_OK:
        return true;

    case USBCCU_INVALID_INTEGER:
        NRF_LOG_INFO("[USB RX] Wrong integer data.");
        snprintf(text_buff, buff_msg_size, "\r\n> Wrong integer passed.\r\n");
        return false;

    case USBCCU_INVALID_ARGS:
        NRF_LOG_INFO("[USB RX] Wrong syntax.");
        snprintf(text_buff, buff_msg_size, "\r\n> Wrong command syntax.\r\n");
        return false;

    case USBCCU_INVALID_RED:
        NRF_LOG_INFO("[USB RX] Red invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Red color code is invalid.\r\n");
        return false;
    case USBCCU_INVALID_GREEN:
        NRF_LOG_INFO("[USB RX] Green invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Green color code is invalid.\r\n");
        return false;
    case USBCCU_INVALID_BLUE:
        NRF_LOG_INFO("[USB RX] Blue invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Blue color code is invalid.\r\n");
        return false;


    case USBCCU_INVALID_HUE:
        NRF_LOG_INFO("[USB RX] Hue invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Hue is invalid.\r\n");
        return false;
    case USBCCU_INVALID_SAT:
        NRF_LOG_INFO("[USB RX] Sat invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Saturation is invalid.\r\n");
        return false;
    case USBCCU_INVALID_BRI:
        NRF_LOG_INFO("[USB RX] Bri invalid.");
        snprintf(text_buff, buff_msg_size, "\r\n> Brightness is invalid.\r\n");
        return false;

    case USBCCU_NAME_TOO_LONG:
        NRF_LOG_INFO("[USB RX] Name is too long.");
        snprintf(text_buff, buff_msg_size, "\r\n> The name is too long, can't exceed %d.\r\n", MAX_NAME_LEN);
        return false;
    case USBCCU_NAME_EXISTS:
        NRF_LOG_INFO("[USB RX] Name is already present.");
        snprintf(text_buff, buff_msg_size, "\r\n> That name is already present.\r\n");
        return false;

    default:
        return false;
    }
    return true;
}


static void handler_rgb(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Set RGB");
    int count = 3;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    int nums[count];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_ints(arr, nums, count)))
        return;


    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_rgb(nums)))
        return;

    NRF_LOG_INFO("[USB RX] RGB color set to R: %d, G: %d, B: %d.", nums[0], nums[1], nums[2]);
    snprintf(text_buff, buff_msg_size, "\r\n>> RGB color set to R: %d, G: %d, B: %d.\r\n", nums[0], nums[1], nums[2]);

    cmd_args_inner_color_union_t clr;
    clr.rgb.r = nums[0];
    clr.rgb.g = nums[1];
    clr.rgb.b = nums[2];
    fill_usb_data(&clr, USB_COLOR_RGB);
}

static void handler_hsv(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Set HSV");
    int count = 3;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    int nums[count];
    if (usbccu_check_ints(arr, nums, count) == USBCCU_INVALID_INTEGER)
    {
        NRF_LOG_INFO("[USB RX] Wrong data.");
        snprintf(text_buff, buff_msg_size, "\r\n> Wrong data passed.\r\n");
        return;
    }
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_hsv(nums)))
        return;

    NRF_LOG_INFO("[USB RX] HSV color set to H: %d, S: %d, V: %d.", nums[0], nums[1], nums[2]);
    snprintf(text_buff, buff_msg_size, "\r\n>> HSV color set to H: %d, S: %d, V: %d.\r\n", nums[0], nums[1], nums[2]);

    cmd_args_inner_color_union_t clr;
    clr.hsv.h = nums[0];
    clr.hsv.s = nums[1];
    clr.hsv.v = nums[2];
    fill_usb_data(&clr, USB_COLOR_HSV);
}

static void handler_get(char* const text_buff, const char * const args, int buff_msg_size)
{
    UNUSED_VARIABLE(args);
    color_rgb_t clr;
    get_rgb(&clr);
    NRF_LOG_INFO("[USB RX] Requested RGB, current: R: %ld, G: %ld, B: %ld.", clr.r, clr.g, clr.b);
    snprintf(text_buff, buff_msg_size, "\r\n>> Current RGB color: R: %ld, G: %ld, B: %ld.\r\n", clr.r, clr.g, clr.b);
}

static void handler_unknown(char* const text_buff, const char * const args, int buff_msg_size)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] Unknown command.");
    snprintf(text_buff, buff_msg_size, "\r\n> Unknown command. Type \"HELP\" to list available commands.\r\n");
}

static void handler_help(char* const text_buff, const char * const args, int buff_msg_size)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] Asking for help.");
    char info[buff_msg_size];
    memset(info, 0, buff_msg_size);

    strcat(info, "\r\n");
    strcat(info, "== Commands ==");
    strcat(info, "\r\n");
    for (int i = 0; i < sizeof(cmdhdls)/sizeof(cmdhdls[0]); i++)
    {
        strcat(info, cmdhdls[i].command);
        strcat(info, " ");
        strcat(info, cmdhdls[i].args);
        strcat(info, "\r\n");
        strcat(info, cmdhdls[i].info);
        strcat(info, "\r\n");
    }
    strcat(info, "Any other text will result in Unknown command message.");
    strcat(info, "\r\n");
    strcat(info, "== End ==");
    strcat(info, "\r\n");

    strcpy(text_buff, info);
}

static void handler_list(char* const text_buff, const char * const args, int buff_msg_size)
{
    UNUSED_VARIABLE(args);
    NRF_LOG_INFO("[USB RX] List requested.");
    if (!colors_names[0].saved)
    {
        NRF_LOG_INFO("[USB RX] No saved colors.");
        snprintf(text_buff, buff_msg_size, "\r\n>> No saved colors.\r\n");
        return;
    }

    NRF_LOG_INFO("[USB RX] Saved colors:");
    snprintf(text_buff, buff_msg_size, "\r\n>> Saved colors:\r\n");

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            return;

        char info[64];
        memset(info, 0, 64);
        switch (colors_names[i].color_type)
        {
            case USB_COLOR_RGB:
                snprintf(info, 64, "%d. %s (RGB), R: %d, G: %d, B: %d\r\n", (i + 1), colors_names[i].name, colors_names[i].color.rgb.r,
                                                                                        colors_names[i].color.rgb.g,
                                                                                        colors_names[i].color.rgb.b);
                break;
            case USB_COLOR_HSV:
                snprintf(info, 64, "%d. %s (HSV), H: %d, S: %d, V: %d\r\n", (i + 1), colors_names[i].name, colors_names[i].color.hsv.h,
                                                                                        colors_names[i].color.hsv.s,
                                                                                        colors_names[i].color.hsv.v);
                break;
            default:
                break;
        }

        NRF_LOG_INFO("[USB RX] %s", info);
        strcat(text_buff, info);
    }
}

static void handler_add_rgb(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Adding RGB record.");
    if (colors_names[MAX_COLORS - 1].saved)
    {
        NRF_LOG_INFO("[USB RX] Max colors present.");
        snprintf(text_buff, buff_msg_size, "\r\n> Max colors saved, can't add new.\r\n");
        return;
    }

    int count = 4;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_name(arr[3], MAX_NAME_LEN)))
        return;

    int nums[count - 1];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_ints(arr, nums, count - 1)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_rgb(nums)))
        return;

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_are_names_equal(colors_names[i].name, arr[3])))
            return;

        if (colors_names[i].saved)
            continue;

        strcpy(colors_names[i].name, arr[3]);
        cmd_args_inner_color_union_t color;
        color.rgb.r = nums[0];
        color.rgb.g = nums[1];
        color.rgb.b = nums[2];
        save_color_data(&(colors_names[i]), &color, USB_COLOR_RGB);
        NRF_LOG_INFO("[USB RX] Saving new RGB color.");
        snprintf(text_buff, buff_msg_size, "\r\n>> Successfully saved new RGB color. Colors saved: %d.\r\n", (i + 1));
        return;
    }
}

static void handler_add_hsv(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Adding HSV record.");
    if (colors_names[MAX_COLORS - 1].saved)
    {
        NRF_LOG_INFO("[USB RX] Max colors present.");
        snprintf(text_buff, buff_msg_size, "\r\n> Max colors saved, can't add new.\r\n");
        return;
    }

    int count = 4;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_name(arr[3], MAX_NAME_LEN)))
        return;

    int nums[count - 1];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_ints(arr, nums, count - 1)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_hsv(nums)))
        return;

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_are_names_equal(colors_names[i].name, arr[3])))
            return;

        if (colors_names[i].saved)
            continue;

        strcpy(colors_names[i].name, arr[3]);
        cmd_args_inner_color_union_t color;
        color.hsv.h = nums[0];
        color.hsv.s = nums[1];
        color.hsv.v = nums[2];

        save_color_data(&(colors_names[i]), &color, USB_COLOR_HSV);
        NRF_LOG_INFO("[USB RX] Saving new HSV color.");
        snprintf(text_buff, buff_msg_size, "\r\n>> Successfully saved new HSV color. Colors saved: %d.\r\n", (i + 1));
        return;
    }
}

static void handler_add_curr(char* const text_buff, const char * const args, int buff_msg_size)
{
    color_rgb_t clr;
    get_rgb(&clr);
    NRF_LOG_INFO("[USB RX] Adding current color record, current: R: %d, G: %d, B: %d.", clr.r, clr.g, clr.b);
    if (colors_names[MAX_COLORS - 1].saved)
    {
        NRF_LOG_INFO("[USB RX] Max colors present.");
        snprintf(text_buff, buff_msg_size, "\r\n> Max colors saved, can't save current.\r\n");
        return;
    }

    int count = 1;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_name(arr[0], MAX_NAME_LEN)))
        return;


    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_are_names_equal(colors_names[i].name, arr[0])))
            return;

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
        colors_names[i].color_type = USB_COLOR_RGB;
        colors_names[i].saved = true;
        NRF_LOG_INFO("[USB RX] Saved current color.");
        snprintf(text_buff, buff_msg_size, "\r\n>> Successfully saved current color. Colors saved: %d.\r\n", (i + 1));
        return;
    }
}

static void handler_del(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Deleting record.");
    int count = 1;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_name(arr[0], MAX_NAME_LEN)))
        return;

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            break;

        if (usbccu_are_names_equal(colors_names[i].name, arr[0]) != USBCCU_OK)
            continue;

        for (int j = i + 1; j < MAX_COLORS; j++)
        {
            strcpy(colors_names[j - 1].name, colors_names[j].name);
            colors_names[j - 1].color = colors_names[j].color;
            colors_names[j - 1].saved = colors_names[j].saved;
            colors_names[j - 1].color_type = colors_names[j].color_type;
        }

        NRF_LOG_INFO("[USB RX] Removed color.");
        snprintf(text_buff, buff_msg_size, "\r\n>> Removed saved color %s.\r\n", arr[0]);
        return;

    }

    NRF_LOG_INFO("[USB RX] No name found.");
    snprintf(text_buff, buff_msg_size, "\r\n> Specified name not found.\r\n");
}

static void handler_set(char* const text_buff, const char * const args, int buff_msg_size)
{
    NRF_LOG_INFO("[USB RX] Setting color by name.");
    int count = 1;
    const char *arr[count];

    char tword[strlen(args)];
    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_get_args(args, tword, arr, count)))
        return;

    if (!parse_usbccu_code(text_buff, buff_msg_size, usbccu_check_name(arr[0], MAX_NAME_LEN)))
        return;

    for (int i = 0; i < MAX_COLORS; i++)
    {
        if (!colors_names[i].saved)
            break;

        if (strcmp(colors_names[i].name, arr[0]) != 0)
            continue;



        fill_usb_data(&(colors_names[i].color), colors_names[i].color_type);

        NRF_LOG_INFO("[USB RX] Set color.");
        snprintf(text_buff, buff_msg_size, "\r\n>> Set current color: %s.\r\n", arr[0]);
        return;

    }

    NRF_LOG_INFO("[USB RX] No such name.");
    snprintf(text_buff, buff_msg_size, "\r\n> Specified name not found.\r\n");
}

void usbc_process_command(char * const buff, char * const command_buff, int buff_msg_size)
{
    char temp[strlen(command_buff)];
    strcpy(temp, command_buff);
    char *prefix = strtok(temp, " ");
    for (int i = 0; i < sizeof(cmdhdls)/sizeof(cmdhdls[0]); i++)
    {
        if (strcmp(cmdhdls[i].command, prefix) == 0)
        {
            cmdhdls[i].handler(buff, command_buff, buff_msg_size);
            return;
        }
    }
    handler_unknown(buff, command_buff, buff_msg_size);
}

void usbc_init(usb_data_t * const usbd, void (*action)(color_rgb_t*))
{
    get_rgb = action;
    usb_data = usbd;
}