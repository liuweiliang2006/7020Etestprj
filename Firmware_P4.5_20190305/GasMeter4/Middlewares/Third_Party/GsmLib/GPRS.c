#include "Sim80x.h" 

#if (_SIM80X_USE_GPRS==1)
//####################################################################################################
//断开socket链接
bool  GPRS_CloseSocket()
{
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			answer = Sim80x_SendAtCommand("AT+QICLOSE\r\n",3000,1,"CLOSE OK");
	}
	else
	{
    	answer = Sim80x_SendAtCommand("AT+CIPCLOSE=1\r\n",3000,1,"CLOSE OK");
	}
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_CIPCLOSE() ---> OK\r\n");
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_CIPCLOSE() ---> ERROR\r\n");
#endif
        return false;
    }
}

//####################################################################################################
//AT+CGATT=1 （Attach or detach from GPRS service, GPRS 附着状态）
//说的简单点，这一步就是让SGSN (服务GPRS节点，你可以把它理解成与基站紧密相连的一台设备，他可能记录你的移动终端的位置，状态等等很多很多信息)
//知道你的存在并且认为你拥有GPRS功能。由于GSM和GPRS用的都是相同的基站，
//所以通常你的MS开启，注册上网络了，你就已经是GSM的一个节点了，可以打电话了。
//但是，如果你想使用GPRS数据业务，你就要附着GPRS服务，这个命令就是干这个的。

//这个是自动执行的,但网络不好的情况需要判断  
//附着到GPRS网络
//设置透传AT+CIPMODE=1,
//APN设置APN:AT+CSTT="CMNET","","" 配套使用
//激活移动场景AT+CIICR，建立无线连接等  配套使用
//执行顺序1
bool  GPRS_StartCGATT(uint8_t tryCnt)
{
    uint8_t answer;
    answer = Sim80x_SendAtCommand("AT+CGATT=1\r\n",4000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
            printf("\r\nGPRS_StartADDGPRS(%d) ---> OK\r\n",tryCnt);
#endif   
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
                printf("\r\nGPRS_StartADDGPRS(%d) ---> ERROR\r\n",tryCnt);
#endif
        return false;
    }
}

//####################################################################################################
//AT+CIPSHUT（ Disconnect wireless connection, Close Moving Scene）
//关闭移动场景，当你执行这个的时候你就进入IP INITIAL 状态，如果你再重新尝试连接的时候你的本地IP地址很可能已经就不是原来的了。
//1. 通常下面提到的前三个命令(CGATT,CGDCONT,CGACT)我们基本不需要使用，因为默认值都符合我们刚刚的要求，而芯片基本把这些事情都做了。
//   可以通过相应的查询命令获得这些信息，如果有不符合的，可以另行处理。
//2. 有些移动的SIMCARD不支持CMNET接入方式，所以要特别小心，当你发现你所有的参数都正确，
//   能够发送短信和打电话，就是无法连接或者连接总是失败，你就该检查你一下你的卡到底支不支持CMNET接入方式。
//   全球通基本两种接入方式都支持，而动感地带和神州行就不一定了。

//3. 如果你查询的状态总是+PDP: DEACT，就是说无法激活，那你应该看看天线是否接牢固或者是否有断线的地方。

//4. 在进入业务操作之前，确保你的模块没有工作在最小功能模式（minimum functionality），你可以通过AT+CFUN查询模块的工作状态。

//5. 在尝试连接一个服务器和某个端口之前，最好先在计算机上用SOCKET工具连接一下，看看是否是通的，
//   有些网络和防火墙很可能会封杀端口的，如果你寻找或调试了一天后，发现是这个问题，唉，大家知道会有多郁闷。
//  （另外SP可以为你的ADSL分配动态或者固定公网IP，有些在ipconfig或者网络设置里显示两个IP，别混淆了）
//   socket 调试工具下载地址：http://www.onlinedown.net/soft/55038.htm

//6. 在尝试连接（AT+CIPSTART）之前，一定要查询（AT+CIPSTATUS）当前状态，一共有下面几种状态
//0 IP INITIAL    				Initializing state
//1 IP START      				Starting State
//2 IP CONFIG     				Configure Scene
//3 IP IND Accept 				Scene Configuration
//4 IP GPRSACT 						Scene Activated
//5 IP STATUS 						Get Local IP Address
//6 TCP/UDP CONNECTING 		establish connection state with SERVER
//7 IP CLOSE 							Connection has been closed
//8 CONNECT OK 						Connection Established Successfully
//9 PDP DEACT 						Detach from GPRS network
//而AT+CIPSTART只在IP_INITIAL、IP_CLOSE、IP_STATUS三种状态下才能正确建立连接，否则返回ERROR。
//所以有时候会连接失败（比如信号不好等原因），而模块通常会进入TCP/UDP CONNECTING状态，就是一直不停的尝试连接，
//而此时如果执行连接命令就会返回错误信息。虽然此时的状态是TCP/UDP CONNECTING，但是实际上很多模块已经不再连接了。
//只不过处在连接状态上而已，这时你最好通过CIPCLOSE或者CIPSHUT命令，回到一个可连接的状态。
//断开移动场景连接,类似于关闭GPRS
//执行顺序2
bool  GPRS_DeactivatePDPContext()
{
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			answer = Sim80x_SendAtCommand("AT+QIDEACT\r\n",5000,2,"\r\nDEACT OK\r\n","\r\nERROR\r\n");
	}
	else
	{
    	answer = Sim80x_SendAtCommand("AT+CIPSHUT\r\n",5000,2,"\r\nSHUT OK\r\n","\r\nERROR\r\n");
	}
    if(answer == 1)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_DeactivatePDPContext() ---> OK\r\n");
				#endif
        return true;
    }
    else
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_DeactivatePDPContext() ---> ERROR\r\n");
				#endif
        return false;
    }
}

//####################################################################################################
//AT+CGDCONT=1,"IP","CMNET" （Define PDP context, 定义PDP 上下文）
//如果打算用PDP（Packet Data Protocol ）传输数据，你就要为他建立一个背景，就是所谓的上下文，
//你得告诉SGSN和GGSN (网关GPRS节点，可以把它理解成一台与SGSN和INTERNET紧密相连的设备，
//它把从SGSN的一些信息处理后发送到INTERNET的服务器同时把服务器的信息处理后再发送给SGSN，
//SGSN再给你的终端设备)一些基本信息，比如你的本地标识（cid），你希望的pdp_type（这里就是IP），
//还有你的接入点信息。这里的接入点是CMNET，实际上他应该是一台服务器，是INTERNET和SGSN的纽带。
//而中国移动公司提供了一个名字CMNET（China Mobile NET），我们通过它就能连接INTERNET了。
//而在国外，这个参数通常是一个域名性质的东西。另外，得特别注意，就是中国移动人为的分出两种就如方式，
//我们刚刚使用的是CMNET，另一种是CMWAP，很明显，这个只能上WAP，但是通过他们的HTTP代理服务器，我们已能够上INTERNET，
//但是仅仅局限于HTTP而已。（有些移动的SIMCARD不支持CMNET接入方式，所以要特别小心，）
//这个可以不执行,如果执行最好在CIPSHUT后执行,貌似模块执行这个后什么也没有干
bool  GPRS_CGDCONT(char *Name)
{
    char str[64];
    uint8_t answer;
    sprintf(str,"AT+CGDCONT=1,\"IP\",\"%s\"\r\n",Name);
    answer = Sim80x_SendAtCommand(str,2000,1,"OK");
    if(answer == 1)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_USEAPN(\"%s\") ---> OK\r\n",Name);
				#endif
        return true;
    }
    else
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_USEAPN(\"%s\") ---> ERROR\r\n",Name);
				#endif
        return false;
    }
}

//####################################################################################################
//AT+CGACT=1,1 （PDP context activate or deactivate, 激活或解除PDP 移动场景）
//前面的定义PDP 上下文其实只是说我们有一种想要使用GPRS通信的想法，并且声明了我们想如何使用，但是这仅仅是个概念，
//如果你真的想付诸实施，就必须让网络知道你的意愿，并且记录你的信息，为你分配资源等等，所以，激活他吧。
//这个命令里的第一个参数，就是我们前面定义的cid标识值，第二个就是激活状态，如果你想激活，就设置1，如果你想解除就设置0。
bool  GPRS_CGACT(char *Name)
{
    char str[64];
    uint8_t answer;
    sprintf(str,"AT+CGACT=1,1\r\n");
    answer = Sim80x_SendAtCommand(str,3000,1,"OK");
    if(answer == 1)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_CGACT(\"%s\") ---> OK\r\n",Name);
				#endif
        return true;
    }
    else
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_CGACT(\"%s\") ---> ERROR\r\n",Name);
				#endif
        return false;
    }
}

//####################################################################################################
//设置是否支持多个连接
//执行顺序3
bool  GPRS_SetMultiConnection(bool Enable)
{
    uint8_t answer;
    if(Enable==true)
        answer = Sim80x_SendAtCommand(" AT+CIPMUX=1\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
    else
        answer = Sim80x_SendAtCommand(" AT+CIPMUX=0\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetMultiConnection(%d) ---> OK\r\n",Enable);
#endif
        Sim80x.GPRS.MultiConnection=Enable;
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetMultiConnection(%d) ---> ERROR\r\n",Enable);
#endif
        return false;
    }
}

//####################################################################################################
//获得APN
//可以获得默认的APN
//执行顺序4
bool  GPRS_GetAPN(char *Name,char *username,char *password)
{
    uint8_t answer;
		if(Sim80x.Modem_Type == Quectel_M26)
		{
			answer = Sim80x_SendAtCommand("AT+QIREGAPP?\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
		}
		else
		{
			answer = Sim80x_SendAtCommand("AT+CSTT?\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
		}
    if(answer==1)
    {
        if(Name!=NULL)
            strcpy(Name,Sim80x.GPRS.APN);
        if(username!=NULL)
            strcpy(username,Sim80x.GPRS.APN_UserName);
        if(password!=NULL)
            strcpy(password,Sim80x.GPRS.APN_Password);
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_GetAPN(%s,%s,%s) <--- OK\r\n",Sim80x.GPRS.APN,Sim80x.GPRS.APN_UserName,Sim80x.GPRS.APN_Password);
				#endif
        return true;
    }
    else
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_GetAPN() <--- ERROR\r\n");
				#endif
        return false;
    }
}

//####################################################################################################
//设置APN
//在关闭连接没有关闭PDP的情况下执行,会返回ERROR,故每次连接前需要推荐先关闭PDP,即执行CIPSHUT,CIICR同样
//执行这个命令后,网络状态为 IP START,可以理解为模块启动了一个线程准备好了连接网络
//执行顺序5
bool  GPRS_SetAPN(char *Name,char *username,char *password)
{
    char str[64];
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			sprintf(str,"AT+QIREGAPP=\"%s\",\"%s\",\"%s\"\r\n",Name,username,password);
	}
	else
	{
    	sprintf(str,"AT+CSTT=\"%s\",\"%s\",\"%s\"\r\n",Name,username,password);
	}
    answer = Sim80x_SendAtCommand(str,1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
    if(answer == 1)
    {
        strcpy(Sim80x.GPRS.APN,Name);
        strcpy(Sim80x.GPRS.APN_UserName,username);
        strcpy(Sim80x.GPRS.APN_Password,password);
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetAPN(\"%s\",\"%s\",\"%s\") ---> OK\r\n",Name,username,password);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetAPN(\"%s\",\"%s\",\"%s\") ---> ERROR\r\n",Name,username,password);
#endif
        return false;
    }
}

//####################################################################################################
//设置pin
//Sim80x_SendAtCommand("AT+CPIN=\"4294\"\r\n",2000,1,"OK");
bool  GPRS_SetPin(char *pin)
{
    char str[64];
    uint8_t answer;
    sprintf(str,"AT+CPIN=\"%s\"\r\n",pin);
    answer = Sim80x_SendAtCommand(str,2000,1,"\r\nOK\r\n");
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetPin(\"%s\") ---> OK\r\n",pin);
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_SetPin(\"%s\") ---> ERROR\r\n",pin);
#endif
        return false;
    }
}

//####################################################################################################
//激活移动场景，建立无线连接
//执行顺序6
//成功后的状态为 STATE: IP GPRSACT
bool  GPRS_StartUpGPRS(void)
{
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			answer = Sim80x_SendAtCommand("AT+QIACT\r\n",5000,2,"\r\nOK\r\n","\r\nERROR\r\n");
	}
	else
	{
    	answer = Sim80x_SendAtCommand("AT+CIICR\r\n",5000,2,"\r\nOK\r\n","\r\nERROR\r\n");
	}
    if(answer == 1)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_StartUpGPRS() ---> OK\r\n");
				#endif
        return true;
    }
    else
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_StartUpGPRS() --->ERROR\r\n");
				#endif
        return false;
    }
}

//####################################################################################################
//获取本地IP,在附着网络,建立场景后
//执行顺序7
//成功后的状态为 STATE: IP STATUS
//然后就可以连接网络了
void  GPRS_GetLocalIP(char *IP)
{
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			answer = Sim80x_SendAtCommand("AT+QILOCIP\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
	}
	else
	{
    	answer = Sim80x_SendAtCommand("AT+CIFSR\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
	}
    if((IP!=NULL) && (answer==1))
        strcpy(IP,Sim80x.GPRS.LocalIP);
#if (_SIM80X_DEBUG==1)
    printf("\r\nGPRS_GetLocalIP(%s) <--- OK\r\n",Sim80x.GPRS.LocalIP);
#endif
}

//####################################################################################################
//设置接收到的数据显示IP等信息,+IPD,3:drr
//执行顺序8
bool  GPRS_ShowGPRSIPD()
{
    uint8_t answer;
	if(Sim80x.Modem_Type == Quectel_M26)
	{
			answer = Sim80x_SendAtCommand("AT+QIHEAD=1\r\n",2000,1,"\r\nOK\r\n");
	}
	else
	{
    	answer = Sim80x_SendAtCommand("AT+CIPHEAD=1\r\n",2000,1,"\r\nOK\r\n");
	}
    if(answer == 1)
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_ShowGPRSIP() ---> OK\r\n");
#endif
        return true;
    }
    else
    {
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_ShowGPRSIP() --->ERROR\r\n");
#endif
        return false;
    }
}

//####################################################################################################
//获取当前联网状态
void  GPRS_GetCurrentConnectionStatus(void)
{
    Sim80x_SendAtCommand("AT+CIPSTATUS\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
#if (_SIM80X_DEBUG==1)
    printf("\r\nGPRS_GetCurrentConnectionStatus() <--- OK\r\n");
#endif
}
//####################################################################################################
bool  GPRS_GetMultiConnection(void)
{
    Sim80x_SendAtCommand(" AT+CIPMUX?\r\n",1000,2,"\r\nOK\r\n","\r\nERROR\r\n");
#if (_SIM80X_DEBUG==1)
    printf("\r\nGPRS_GetMultiConnection(%d) <--- OK\r\n",Sim80x.GPRS.MultiConnection);
#endif
    return Sim80x.GPRS.MultiConnection;
}

//####################################################################################################
//默认的联网函数,未经过确认
bool  GPRS_ConnectToNetwork(char *Name,char *username,char *password,bool EnableMultiConnection)
{
    if(GPRS_DeactivatePDPContext()==false)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_ConnectToNetwork() ---> ERROR\r\n");
				#endif
        return false;
    }
    GPRS_SetMultiConnection(EnableMultiConnection);
    if(GPRS_SetAPN(Name,username,password)==false)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_ConnectToNetwork() ---> ERROR\r\n");
				#endif
        return false;
    }
    if(GPRS_StartUpGPRS()==false)
    {
				#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_ConnectToNetwork() ---> ERROR\r\n");
				#endif
        return false;
    }
    GPRS_GetLocalIP(NULL);
		#if (_SIM80X_DEBUG==1)
    printf("\r\nGPRS_ConnectToNetwork() ---> OK\r\n");
		#endif
    return true;
}

//####################################################################################################
bool  GPRS_HttpGet(char *URL)
{
    uint16_t timeout;
    uint8_t answer;
    char str[100];
    answer = Sim80x_SendAtCommand("AT+SAPBR=3,1,\"Contype\",\"GPRS\"\r\n",1000,1,"\r\nOK\r\n");
    if(answer!=1)
        goto Error;
    answer = Sim80x_SendAtCommand("AT+SAPBR=1,1\r\n",10000,1,"\r\nOK\r\n");
    if(answer!=1)
        goto Error;
    answer = Sim80x_SendAtCommand("AT+HTTPINIT\r\n",1000,1,"\r\nOK\r\n");
    if(answer!=1)
        goto Error;
    answer = Sim80x_SendAtCommand("AT+HTTPPARA=\"CID\",1\r\n",1000,1,"\r\nOK\r\n");
    if(answer!=1)
        goto Error;

    snprintf(str,sizeof(str),"AT+HTTPPARA=\"URL\",\"%s\"\r\n",URL);
    answer = Sim80x_SendAtCommand(str,10000,2,"\r\nOK\r\n","\r\n+CME ERROR");
    if(answer!=1)
        goto Error;

    Sim80x.GPRS.HttpAction.ResultCode=0;
    answer = Sim80x_SendAtCommand("AT+HTTPACTION=0\r\n",1000,1,"\r\nOK\r\n");
    if(answer!=1)
        goto Error;
    timeout=0;
    while(Sim80x.GPRS.HttpAction.ResultCode==0)
    {
        osDelay(1000);
        timeout++;
        if(timeout == 120)
            goto Error;
    }
    if(Sim80x.GPRS.HttpAction.ResultCode==200)
    {
        Sim80x.GPRS.HttpAction.TransferStartAddress=0;
        for(uint32_t start=0 ; start<Sim80x.GPRS.HttpAction.DataLen ; start+=256 )
        {
            Sim80x.GPRS.HttpAction.CopyToBuffer=0;
            sprintf(str,"AT+HTTPREAD=%d,%d\r\n",start,256);
            Sim80x_SendString(str);
            timeout=0;
            while(Sim80x.GPRS.HttpAction.CopyToBuffer==0)
            {
                osDelay(10);
                timeout++;
                if(timeout == 100)
                    goto Error;
            }
            Sim80x.GPRS.HttpAction.TransferStartAddress =  start;
            GPRS_UserHttpGetAnswer(Sim80x.GPRS.HttpAction.Data,Sim80x.GPRS.HttpAction.TransferStartAddress,Sim80x.GPRS.HttpAction.TransferDataLen);
        }
        answer = Sim80x_SendAtCommand("AT+HTTPTERM\r\n",1000,1,"\r\nOK\r\n");
        answer = Sim80x_SendAtCommand("AT+SAPBR=0,1\r\n",1000,1,"\r\nOK\r\n");
#if (_SIM80X_DEBUG==1)
        printf("\r\nGPRS_HttpGet(%s) ---> OK\r\n",URL);
#endif
        return true;
    }
Error:
    answer = Sim80x_SendAtCommand("AT+HTTPTERM\r\n",1000,1,"\r\nOK\r\n");
    answer = Sim80x_SendAtCommand("AT+SAPBR=0,1\r\n",1000,1,"\r\nOK\r\n");
#if (_SIM80X_DEBUG==1)
    printf("\r\nGPRS_HttpGet(%s) ---> ERROR\r\n",URL);
#endif
    return false;
}
#endif
