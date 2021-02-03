#include <string.h>
#include <stdio.h>
#include <stdbool.h>

#include <switch.h>

#define IRAM_PAYLOAD_MAX_SIZE 0x24000
#define IRAM_PAYLOAD_BASE 0x40010000

static alignas(0x1000) u8 g_reboot_payload[IRAM_PAYLOAD_MAX_SIZE];
static alignas(0x1000) u8 g_ff_page[0x1000];
static alignas(0x1000) u8 g_work_page[0x1000];

void do_iram_dram_copy(void *buf, uintptr_t iram_addr, size_t size, int option) {
    memcpy(g_work_page, buf, size);

    SecmonArgs args = {0};
    args.X[0] = 0xF0000201;             /* smcAmsIramCopy */
    args.X[1] = (uintptr_t)g_work_page;  /* DRAM Address */
    args.X[2] = iram_addr;              /* IRAM Address */
    args.X[3] = size;                   /* Copy size */
    args.X[4] = option;                 /* 0 = Read, 1 = Write */
    svcCallSecureMonitor(&args);

    memcpy(buf, g_work_page, size);
}

void copy_to_iram(uintptr_t iram_addr, void *buf, size_t size) {
    do_iram_dram_copy(buf, iram_addr, size, 1);
}

void copy_from_iram(void *buf, uintptr_t iram_addr, size_t size) {
    do_iram_dram_copy(buf, iram_addr, size, 0);
}

static void clear_iram(void) {
    memset(g_ff_page, 0xFF, sizeof(g_ff_page));
    for (size_t i = 0; i < IRAM_PAYLOAD_MAX_SIZE; i += sizeof(g_ff_page)) {
        copy_to_iram(IRAM_PAYLOAD_BASE + i, g_ff_page, sizeof(g_ff_page));
    }
}

static void reboot_to_payload(void) {
    clear_iram();

    for (size_t i = 0; i < IRAM_PAYLOAD_MAX_SIZE; i += 0x1000) {
        copy_to_iram(IRAM_PAYLOAD_BASE + i, &g_reboot_payload[i], 0x1000);
    }

    splSetConfig((SplConfigItem)65001, 2);
}

int main(int argc, char **argv)
{
    consoleInit(NULL);

    padConfigureInput(8, HidNpadStyleSet_NpadStandard);

    PadState pad;
    padInitializeAny(&pad);
    
    bool can_reboot = true;
    bool can_reboot_ams = true;
    bool can_reboot_hkt = true;
    
  //printf(" ______        _                                     \n");
  //printf("(_____ \      | |                 _        _         \n");
  //printf(" _____) )_____| |__   ___   ___ _| |_    _| |_ ___   \n");
  //printf("|  __  /| ___ |  _ \ / _ \ / _ (_   _)  (_   _) _ \  \n");
  //printf("| |  \ \| ____| |_) ) |_| | |_| || |_     | || |_| | \n");
  //printf("|_|   |_|_____)____/ \___/ \___/  \__)     \__)___/  \n");

  //printf(" ______             _                 _\n");
  //printf("(_____ \           | |               | |\n");
  //printf(" _____) )____ _   _| | ___  _____  __| |\n");
  //printf("|  ____(____ | | | | |/ _ \(____ |/ _  |\n");
  //printf("| |    / ___ | |_| | | |_| / ___ ( (_| |\n");
  //printf("|_|    \_____|\__  |\_)___/\_____|\____|\n");
  //printf("             (____/\n");


  //printf(" _______                                _\n");
  //printf("(___ ___)                              | |\n");
  //printf("   | | ____ ___  ____   ___   ___ ____ | |__  _____  ____ _____\n");
  //printf("   | |/ ___) _ \|  _ \ / _ \ /___)  _ \|  _ \| ___ |/ ___) ___ |\n");
  //printf("   | | |  | |_| | |_| | |_| |___ | |_| | | | | ____| |   | ____|\n");
  //printf("   |_|_|   \___/|  __/ \___/(___/|  __/|_| |_|_____)_|   |_____)\n");
  //printf("                |_|              |_|\n");

  //printf(" _______          _      \n");
  //printf("( ______)        | |     \n");
  //printf("| |___ __     __ | |  _  \n");
  //printf("|  ___) _ \ / ___) |_/ ) \n");
  //printf("| |  | |_| | |   |  _ (  \n");
  //printf("|_|   \___/|_|   |_| \_) \n");

    printf(" _______                                _\n");
    printf("(___ ___)                              | |\n");
    printf("   | | ____ ___  ____   ___   ___ ____ | |__  _____  ____ _____\n");
    printf("   | |/ ___) _ \\|  _ \\ / _ \\ /___)  _ \\|  _ \\| ___ |/ ___) ___ |\n");
    printf("   | | |  | |_| | |_| | |_| |___ | |_| | | | | ____| |   | ____|\n");
    printf("   |_|_|   \\___/|  __/ \\___/(___/|  __/|_| |_|_____)_|   |_____)\n");
    printf(" _______        |_|              |_|\n");
    printf("( ______)         _ v1.1\n");
    
    Result rc = splInitialize();
    if (R_FAILED(rc)) {
        printf("| |___ __     __ | |     Failed to initialize spl: 0x%x\n", rc);
        printf("|  ___) _ \\ / ___) |_/ )\n");
        printf("| |  | |_| | |   |  _ (  \n");
        printf("|_|   \\___/|_|   |_| \\_) \n");
        can_reboot = false;
    } else {
        FILE *f = fopen("sdmc:/atmosphere/fusee-primary.bin", "rb");
        if (f == NULL) {
            //printf("Failed to open atmosphere/fusee-primary.bin!\nRebooting to Atmosphere disabled\n");
            printf("| |___ __     __ | |  _  Failed to open atmosphere/fusee-primary.bin.\n");
            can_reboot_ams = false;
        }
        else {
            printf("| |___ __     __ | |  _  Press [A], [B], [X], or [Y] to reboot to atmosphere\n");
        }
        
        f = fopen("sdmc:/atmosphere/hekate.bin", "rb");
        if (f == NULL) {
            printf("|  ___) _ \\ / ___) |_/ ) Failed to open atmosphere/hekate.bin.\n");
            can_reboot_hkt = false;
        }
        else {
            printf("|  ___) _ \\ / ___) |_/ ) Press any D-pad button to reboot to hekate\n");
        }
        
        fclose(f);
    }

    printf("| |  | |_| | |   |  _ (  Press [+] or [-] to exit\n");
    printf("|_|   \\___/|_|   |_| \\_)\n");

    // Main loop
    while(appletMainLoop())
    {
        padUpdate(&pad);
        u64 kDown = padGetButtonsDown(&pad);

        if (kDown & (HidNpadButton_Minus | HidNpadButton_Plus)) {
            break;
        }
        else if (kDown && can_reboot) {
            switch (kDown) {
                case HidNpadButton_Up:
                case HidNpadButton_Down:
                case HidNpadButton_Left:
                case HidNpadButton_Right:
                    if (can_reboot_hkt) {                        
                        printf("Rebooting to Hekate");
                        svcSleepThread(2500000000ull);
                        FILE *hkt = fopen("sdmc:/atmosphere/hekate.bin", "rb");
                        fread(g_reboot_payload, 1, sizeof(g_reboot_payload), hkt);
                        fclose(hkt);
                        reboot_to_payload();
                    }
                    break;
                case HidNpadButton_A:
                case HidNpadButton_B:
                case HidNpadButton_X:
                case HidNpadButton_Y:
                    if (can_reboot_ams) {                        
                        printf("Rebooting to Atmosphere");
                        svcSleepThread(2500000000ull);
                        FILE *ams = fopen("sdmc:/atmosphere/fusee-primary.bin", "rb");
                        fread(g_reboot_payload, 1, sizeof(g_reboot_payload), ams);
                        fclose(ams);
                        reboot_to_payload();
                    }
                    break;
            }
        }
        
        consoleUpdate(NULL);
    }

    if (can_reboot) {
        splExit();
    }

    consoleExit(NULL);
    return 0;
}

