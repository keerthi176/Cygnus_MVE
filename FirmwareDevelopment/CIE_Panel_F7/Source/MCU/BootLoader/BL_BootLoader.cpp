//__attribute__((section(".bootloader"), noinline)) 

int BootLoader(void)
{
	return (int) &BootLoader;
}

