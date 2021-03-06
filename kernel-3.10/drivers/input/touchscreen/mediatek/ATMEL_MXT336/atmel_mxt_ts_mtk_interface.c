const static unsigned int mxts_keys[NUM_KEY_TYPE][MAX_KEYS_SUPPORTED_IN_DRIVER] = {
	//T15_T97_KEY
	{KEY_BACK,KEY_HOMEPAGE,KEY_MENU},
	//T19_KEY,
	{KEY_POWER},
	//T24_KEY,
	{KEY_F8},
	//T61_KEY,
	{KEY_F9},
	//T81_KEY,
	{KEY_F8,KEY_F9},
	//T92_KEY,
	{KEY_F8,KEY_F9,KEY_F10,KEY_F11},
	//T93_KEY,
	{KEY_F9},
	//T99_KEY,
	{KEY_F10},
	//T115_KEY,
	{KEY_F11},
	//T116_KEY,
	{KEY_F12}
};

const static u8 mxts_num_keys[NUM_KEY_TYPE] = {
	//T15_T97_KEY
	5,
	//T19_KEY,
	0,
	//T24_KEY,
	1,
	//T61_KEY,
	1,
	//T81_KEY,
	2,
	//T92_KEY,
	4,
	//T93_KEY,
	1,
	//T99_KEY,	
	1,
	//T115_KEY,
	1,
	//T116_KEY,
	1,
};

static struct mxt_config_info mxt_config_array[1] = {
	{
		.self_chgtime_max = 120,
		.self_chgtime_min = 60,
	},
};
static struct mxt_platform_data mxt_platform_data = {
	.irqflags = IRQF_TRIGGER_LOW/*IRQF_TRIGGER_FALLING*/,  //the flag here should be matched with board_init_irq() irq triggle method
	.num_keys = mxts_num_keys,
	.keymap = mxts_keys,
#if defined(CONFIG_MXT_REPORT_VIRTUAL_KEY_SLOT_NUM)
	.max_y_t = 1919,     //Max value of Touch AA, asix more than this value will process by VirtualKeyHit
	.vkey_space_ratio = {5,8,15,10},
#endif
#if defined(CONFIG_MXT_SELFCAP_TUNE)
	.config_array = mxt_config_array,
#endif
};

static struct i2c_board_info __initdata 
	mxt_i2c_tpd={ 
		I2C_BOARD_INFO("atmel_mxt_ts", 0x4a),
		.platform_data = &mxt_platform_data,
				/*.irq = 0*/};

static int  tpd_probe(struct i2c_client *client,
				   const struct i2c_device_id *id) //__devinit
{
	int ret;
	
	printk("mxt tpd_probe\n");

	ret = mxt_probe(client,id);
	if(ret){
		printk("mxt_probe failed\n");
		return ret;
	}

	tpd_load_status = 1;

	return 0;
}

static int tpd_remove(struct i2c_client *client) //__devexit 
{
	tpd_load_status = 0;

	printk("mxt tpd_remove\n");	

	return mxt_remove(client);

	
}

static struct i2c_driver tpd_driver = {
	.driver = {
		.name	= "atmel_mxt_ts",
		.owner	= THIS_MODULE,
#if !(defined(CONFIG_HAS_EARLYSUSPEND) || defined(CONFIG_FB_PM))
		.pm	= &mxt_pm_ops,
#endif
	},
	.probe		= tpd_probe,
	.remove		= tpd_remove,//__devexit_p(),
	.shutdown	= mxt_shutdown,
	.id_table	= mxt_id,
};

static int tpd_local_init(void)
{
	printk("Atmel MXT I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);

	if(i2c_add_driver(&tpd_driver)!=0) {
		printk("mxt error unable to add i2c driver.\n");
		return -1;
	}

	if(tpd_load_status == 0) {  // disable auto load touch driver for linux3.0 porting
		printk("mxt atmel add error touch panel driver.\n");
		i2c_del_driver(&tpd_driver);
		return -1;
	}

	printk("%s, success %d\n", __FUNCTION__, __LINE__);
	tpd_type_cap = 1;
	
	return 0;
}

static void tpd_suspend(struct early_suspend *h)
{
	// here you should call mxt_early_suspend() above if you don't use stand power interface
	// you can refer to below code or you self mind
#if !defined(CONFIG_HAS_EARLYSUSPEND)
	struct mxt_data *data = mxt_g_data;

	printk("[mxt] tpd_suspend\n");

	if(data)
		mxt_suspend(&data->client->dev);
#endif  

}

static void tpd_resume(struct early_suspend *h)
{
	// here you should call mxt_late_resume() above if you don't use stand power interface
	// you can refer to below code or you self mind
#if !defined(CONFIG_HAS_EARLYSUSPEND)	
	struct mxt_data *data = mxt_g_data;
	printk("[mxt] tpd_resume\n");
	
	if(data)
		mxt_resume(&data->client->dev);
#endif
}

static struct tpd_driver_t tpd_device_driver = {
	.tpd_device_name = "atmel_mxt_ts",
	.tpd_local_init = tpd_local_init,
	.suspend =tpd_suspend,
	.resume = tpd_resume,
	//.tpd_have_button = 0,
};
 /* called when loaded into kernel */
static int __init tpd_driver_init(void) 
{
	printk("mxt tpd_driver_init\n");
 
	i2c_register_board_info(2, &mxt_i2c_tpd, 1);
	
	if(tpd_driver_add(&tpd_device_driver) < 0)
		printk("tpd_driver_init failed\n");

	return 0;
}

/* should never be called */
static void __exit tpd_driver_exit(void) 
{
	printk("mxt tpd_driver_exit\n");
	tpd_driver_remove(&tpd_device_driver);
}
 
module_init(tpd_driver_init);
module_exit(tpd_driver_exit);
