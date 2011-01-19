#include <pspsdk.h>
#include <pspkernel.h>

#include <string.h>
#include "systemctrl.h"
#include "systemctrl_se.h"
#include "printk.h"

SEConfig conf;

#define CONFIG_MAGIC 0x47434553

int GetConfig(SEConfig *config)
{
	SceUID fd;
	u32 k1;
   
	k1 = pspSdkSetK1(0);
	memset(config, 0, sizeof(*config));
	fd = sceIoOpen("flash1:/config.se", PSP_O_RDONLY, 0644);

	if (fd < 0) {
		pspSdkSetK1(k1);

		return -1;
	}

	if (sceIoRead(fd, config, sizeof(*config)) != sizeof(*config)) {
		sceIoClose(fd);
		pspSdkSetK1(k1);

		return -2;
	}

	pspSdkSetK1(k1);

	return 0;
}

int SetConfig(SEConfig *config)
{
	u32 k1;
	SceUID fd;

	k1 = pspSdkSetK1(0);
	sceIoRemove("flash1:/config.se");
	fd = sceIoOpen("flash1:/config.se", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0) {
		pspSdkSetK1(k1);

		return -1;
	}

	config->magic = CONFIG_MAGIC;

	if (sceIoWrite(fd, config, sizeof(*config)) != sizeof(*config)) {
		sceIoClose(fd);
		pspSdkSetK1(k1);

		return -1;
	}

	pspSdkSetK1(k1);

	return 0;
}

int sctrlSESetConfigEx(SEConfig *config, int size)
{
	u32 k1;
	SceUID fd;
	int read;
   
	k1 = pspSdkSetK1(0);
	fd = sceIoOpen("flashfat1:/config.se", PSP_O_WRONLY | PSP_O_CREAT | PSP_O_TRUNC, 0777);

	if (fd < 0) {
		pspSdkSetK1(k1);

		return -1;
	}

	config->magic = CONFIG_MAGIC;
	read = sceIoRead(fd, config, size);

	if (read != size) {
		sceIoClose(fd);
		pspSdkSetK1(k1);

		return -1;
	}

	sceIoClose(fd);
	pspSdkSetK1(k1);

	return 0;
}

int sctrlSEGetConfigEx(SEConfig *config, int size)
{
	int read;
	u32 k1;
	SceUID fd;

	read = -1;
	k1 = pspSdkSetK1(0);
	memset(config, 0, size);
	fd = sceIoOpen("flashfat1:/config.se", PSP_O_RDONLY, 0666);

	if (fd > 0) {
		read = sceIoRead(fd, config, size);
		sceIoClose(fd);
		pspSdkSetK1(k1);
		
		return (read == size) ? 0 : -2;
	}

	pspSdkSetK1(k1);

	return -1;
}

int sctrlSESetConfig(SEConfig *config)
{
	return sctrlSESetConfigEx(config, sizeof(*config));
}

int sctrlSEGetConfig(SEConfig *config)
{
	return sctrlSEGetConfigEx(config, sizeof(*config));
}

void load_default_conf(SEConfig *config)
{
	memset(config, 0, sizeof(*config));
	config->magic = CONFIG_MAGIC;
	config->umdmode = MODE_MARCH33;
	config->usbcharge = 1;
	config->machidden = 1;
}

void load_config(void)
{
	int ret;
	
	ret = GetConfig(&conf);

	if (ret != 0) {
		load_default_conf(&conf);
		SetConfig(&conf);
	}
}