/************************************************************
 *
 * Copyright (c) 2014-2018 AndyWang (lovegangwang@gmail.com)
 *
 * FileName: main.c
 * Author: Andy  Version: 0.01  Date: 2014-12-11
 * Description:This file is the main of this program
 * Others: NULL
 * Function List:
 *   1.
 * History:
 *   1.Date:
 *     Author:
 *     Modification:
 *   2. ...
 *
 ************************************************************/

#include <pjsua-lib/pjsua.h>
#include <signal.h>

#define THIS_FILE	"main.c"


// These are defined in pjsua_app.c.
extern pj_bool_t app_restart;
pj_status_t app_init(int argc, char *argv[]);
pj_status_t app_main(void);
pj_status_t app_destroy(void);


/************************************************************
 * Function: setup_socket_signal
 * Description: This function is call the signal of the linux 
 * which is can handle the ctrl-c and keep the sock close safe.
 * Input : NULL
 * Output: NULL
 * Return: NULL
 * Others: NULL
 * History:
 *   1.Date:
 *     Author:
 *     Modification:
 *   2. ...
 ***********************************************************/
static void setup_socket_signal()
{
    signal(SIGPIPE, SIG_IGN);
}

/************************************************************
 * Function: main_func
 * Description: This function is the main of this program
 * Input : argc, argv
 * Output: NULL
 * Return: 1 or 0
 * Others: NULL
 * History:
 *   1.Date:
 *     Author:
 *     Modification:
 *   2. ...
 ***********************************************************/
static int main_func(int argc, char *argv[])
{
    setup_socket_signal();

    do 
	{
		app_restart = PJ_FALSE;

		if (app_init(argc, argv) != PJ_SUCCESS)
		{
		    PJ_LOG(1,(THIS_FILE,("app_init error!")));
			return 1;
		    
		}
		    

		app_main();
		app_destroy();

		/* This is on purpose */
		app_destroy();
    } while (app_restart);

    return 0;
}

int main(int argc, char *argv[])
{
    return pj_run_app(&main_func, argc, argv, 0);
}
