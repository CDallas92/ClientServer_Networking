#include <winsock2.h>
#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <iostream>

#pragma comment(lib,"ws2_32.lib")

//SetUp
#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104
#define BUFFERSIZE		(100)
#define PACKETSIZE (100)

struct MyPacket_t
{
	bool W;
	bool A;
	bool S;
	bool D;

	bool ballL;
	bool ballR;

	MyPacket_t()
	{
		W = false;
		A = false;
		S = false;
		D = false;
	
		ballL = false;
		ballR = false;
	}
};


char Buffer[BUFFERSIZE];
//Create Packet
MyPacket_t MyPacket;

//Declare Functions 
void PacketDataReading(HWND);
void ReDraws(HWND);
void moveBall(HWND);

HDC hdc;

//Ball Bitmap
static HBITMAP bmpSource = NULL;
static HDC hdcSource = NULL;
PAINTSTRUCT ps;
HDC hdcDestination;
//Face Bitmap
static HBITMAP bmpSource2 = NULL;
static HDC hdcSource2 = NULL;
PAINTSTRUCT ps2;
HDC hdcDestination2;
//Ball Bitmap
float BmpX = 15.0f;
//Face Bitmap
float FaceX = 300.0f;
float FaceY = 100.0f;

int nPort=5555;

HWND hEditIn=NULL;
HWND hEditOut=NULL;
SOCKET Socket=NULL;
sockaddr sockAddrClient;

LRESULT CALLBACK WinProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nShowCmd)
{
	WNDCLASSEX wClass;
	ZeroMemory(&wClass,sizeof(WNDCLASSEX));
	wClass.cbClsExtra=NULL;
	wClass.cbSize=sizeof(WNDCLASSEX);
	wClass.cbWndExtra=NULL;
	wClass.hbrBackground=(HBRUSH)COLOR_WINDOW;
	wClass.hCursor=LoadCursor(NULL,IDC_ARROW);
	wClass.hIcon=NULL;
	wClass.hIconSm=NULL;
	wClass.hInstance=hInst;
	wClass.lpfnWndProc=(WNDPROC)WinProc;
	wClass.lpszClassName="Window Class";
	wClass.lpszMenuName=NULL;
	wClass.style=CS_HREDRAW|CS_VREDRAW;

	if(!RegisterClassEx(&wClass))
	{
		int nResult=GetLastError();
		MessageBox(NULL,
			"Window class creation failed\r\nError code:",
			"Window Class Failed",
			MB_ICONERROR);
	}

	HWND hWnd=CreateWindowEx(NULL,
			"Window Class",
			"Winsock Async Server",
			WS_OVERLAPPEDWINDOW,
			200,
			200,
			640,
			480,
			NULL,
			NULL,
			hInst,
			NULL);

	if(!hWnd)
	{
		int nResult=GetLastError();

		MessageBox(NULL,
			"Window creation failed\r\nError code:",
			"Window Creation Failed",
			MB_ICONERROR);
	}

    ShowWindow(hWnd,nShowCmd);

	MSG msg;

	while(GetMessage(&msg,NULL,0,0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		ReDraws(hWnd);
		moveBall(hWnd);
	}

	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{
	switch(msg)
    {
		case WM_COMMAND:
			switch(LOWORD(wParam))
            {
				case IDC_MAIN_BUTTON:
				{

				}
				break;
			}
			break;
		case WM_CREATE: 
		{


		 //Load Bitmap(Ball) At Start Instead Of Constant Redrawing
		 bmpSource = (HBITMAP)LoadImage(NULL, "imageone.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
         hdcSource = CreateCompatibleDC(GetDC(0));
		 SelectObject(hdcSource, bmpSource);
	

		  //Load Bitmap2(Face) At Start Instead Of Constant Redrawing
		  bmpSource2 = (HBITMAP)LoadImage(NULL, "autoBall.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
          hdcSource2 = CreateCompatibleDC(GetDC(0));
          SelectObject(hdcSource2, bmpSource2);
		
			WSADATA WsaDat;
			int nResult=WSAStartup(MAKEWORD(2,2),&WsaDat);
			if(nResult!=0)
			{
				MessageBox(hWnd,
					"Winsock initialization failed",
					"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			Socket=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
			if(Socket==INVALID_SOCKET)
			{
				MessageBox(hWnd,
					"Socket creation failed",
					"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			SOCKADDR_IN SockAddr;
			SockAddr.sin_port=htons(nPort);
			SockAddr.sin_family=AF_INET;
			SockAddr.sin_addr.s_addr=htonl(INADDR_ANY);

			if(bind(Socket,(LPSOCKADDR)&SockAddr,sizeof(SockAddr))==SOCKET_ERROR)
			{
				MessageBox(hWnd,"Unable to bind socket","Error",MB_OK);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			nResult=WSAAsyncSelect(Socket,
					hWnd,
					WM_SOCKET,
					(FD_CLOSE|FD_ACCEPT|FD_READ));
			if(nResult)
			{
				MessageBox(hWnd,
					"WSAAsyncSelect failed",
					"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			if(listen(Socket,(1))==SOCKET_ERROR)
			{
				MessageBox(hWnd,
					"Unable to listen!",
					"Error",
					MB_OK);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
		}
		break;

		case WM_PAINT:
			{
				//Draw Ball
				hdcDestination = BeginPaint(hWnd, &ps);
				BitBlt(hdcDestination, BmpX, 10, 500, 500, hdcSource, 0, 0, SRCCOPY);
				////Hide The Trail -- Drawing White Version Just After
				BitBlt(hdcDestination, BmpX-32.0f, 10, 500, 500, hdcSource, 0, 0, MERGEPAINT);	
				BitBlt(hdcDestination, BmpX+32.0f, 10, 500, 500, hdcSource, 0, 0, MERGEPAINT);
				////End The Painting
				EndPaint(hWnd, &ps);
			
				//Updates Drawn Bitmaps  
				InvalidateRect(hWnd, NULL, FALSE);

			   //Draw Face
			   hdcDestination2 = BeginPaint(hWnd, &ps2);
			   BitBlt(hdcDestination2, FaceX, FaceY, 500, 500, hdcSource2, 0, 0, SRCCOPY);
			   //End The Painting
			   EndPaint(hWnd, &ps2);

			}
		break;

		case WM_DESTROY:
		{
			PostQuitMessage(0);
			shutdown(Socket,SD_BOTH);
			closesocket(Socket);
			WSACleanup();
			return 0;
		}
		break;

		case WM_SOCKET:
		{
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_READ:
				{
					PacketDataReading(hWnd);
				}
				break;

				case FD_CLOSE:
				{
					//Connection Closed Message
					MessageBox(hWnd,
						"Client closed connection",
						"Connection closed!",
						MB_ICONINFORMATION|MB_OK);
					closesocket(Socket);
					SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				}
				break;

				case FD_ACCEPT:
				{
					int size=sizeof(sockaddr);
					Socket=accept(wParam,&sockAddrClient,&size);                
					if (Socket==INVALID_SOCKET)
					{
						int nret = WSAGetLastError();
						WSACleanup();
					}
				
				}
				break;
    			}   
			}
		}
    
    return DefWindowProc(hWnd,msg,wParam,lParam);
}

void PacketDataReading(HWND hWnd)
{
		char Buffer2[PACKETSIZE], * pBuffer;
		pBuffer = Buffer2;
	
		//Read Packet Data
		recv(Socket, Buffer2, sizeof(MyPacket_t), 0);

		//Check if UpKey Pressed - W = Forward WASD = Up
		if(*pBuffer == 'W')
		{
			if(FaceY >= 0)
			{
				MyPacket.S = false;
				MyPacket.W = true;
			}
		}

		//Check if Down Key Pressed - S = Back WASD = Down
		if(*pBuffer == 'S')
		{
			if(FaceY <= 370)
			{
				MyPacket.W = false;
				MyPacket.S = true;
			}
		}

			//Check if Left Key Pressed - A = Left WASD
		if(*pBuffer == 'A')
		{
			if(FaceX >= 0)
			{
				MyPacket.D = false;
				MyPacket.A = true;
			}
		}
				
		//Check if Right Key Pressed - D = Left WASD
		if(*pBuffer == 'D')
		{
			if(FaceX <= 480)
			{
				MyPacket.A = false;
				MyPacket.D = true;
			}
		}

		//Stop Movement Down If No Data Received
		if(!(*pBuffer == 'S'))
		{
			MyPacket.S = false;
		}

		//Stop Movement Up If No Data Received.
		if(!(*pBuffer == 'W'))
		{
			MyPacket.W = false;
		}

		//Stop Movement Left If No Data Received.
		if(!(*pBuffer == 'A'))
		{
			MyPacket.A = false;
		}

		//Stop Movement Right If No Data Received.
		if(!(*pBuffer == 'D'))
		{
			MyPacket.D = false;
		}


		//Ball Left & Right
		//left as True Received - Move Replica Left
		if(*pBuffer == 'J')
		{
			if(BmpX >= 0)
			{
				MyPacket.ballR = false;
				MyPacket.ballL = true;
			}
		}

		//Right As True Receieved - Move Replica Right
				if(*pBuffer == 'K')
		{
			if(BmpX <= 600)
			{
				MyPacket.ballL = false;
				MyPacket.ballR = true;
			}
		}

		//Stop Ball
		//Stop The Movement If No Data Received
		if(!(*pBuffer == 'J'))
		{
			MyPacket.ballL = false;
		}

		//Stop Movement If No Data Received.
		if(!(*pBuffer == 'K'))
		{
			MyPacket.ballR = false;
		}

}

void ReDraws(HWND hWnd)
{
	
	//If Up Received, Move The Face Up
	if(MyPacket.W == true)
	{
		FaceY = FaceY - 0.5f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	//If Down Received, Move The Face Down
	if(MyPacket.S == true)
	{
		FaceY = FaceY + 0.5f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	//If Left Received, Move The Face Left
	if(MyPacket.A == true)
	{
		FaceX = FaceX - 0.5f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	//If Right Received, Move The Face Right
	if(MyPacket.D == true)
	{
		FaceX = FaceX + 0.5f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

}

void moveBall(HWND hWnd)
{
	//If Left True Received, Move Ball Left
	if(MyPacket.ballL == true)
	{
		if(BmpX >= 0)
		{
		BmpX = BmpX - 0.08f;
		InvalidateRect(hWnd, NULL, FALSE);
		}
	}

	//If Right True Received, Move Ball Right
	if(MyPacket.ballR == true)
	{
		if(BmpX <= 580)
		{
		BmpX = BmpX + 0.08f;
		InvalidateRect(hWnd, NULL, FALSE);	
		}
	}
}
