#include <winsock2.h>
#include <windows.h>

#pragma comment(lib,"ws2_32.lib")

//Setup 
#define IDC_EDIT_IN		101
#define IDC_EDIT_OUT		102
#define IDC_MAIN_BUTTON		103
#define WM_SOCKET		104

char *szServer="localhost";
int nPort=5555;

#define PACKETSIZE (100)
#define BUFFERSIZE (100)

char Buffer[BUFFERSIZE];


//Ball Bitmap
float BmpX = 15.0f;
float BmpY = 10.0f;
//Automate Ball 
bool left = false;
bool right = true;
//Face Bitmap
float FaceX = 300.0f;
float FaceY = 100.0f;
//Ball 
static HBITMAP bmpSource = NULL;
static HDC hdcSource = NULL;
//Face
static HBITMAP bmpSource2 = NULL;
static HDC hdcSource2 = NULL;
//Paint
PAINTSTRUCT ps;
PAINTSTRUCT ps2;
//Images
HDC hdcDestination;
HDC hdcDestination2;

//press count
int presscount =0;

//Declare Functions
void sendMove();
void keyPress(HWND);

HWND hEditIn=NULL;
HWND hEditOut=NULL;
SOCKET Socket=NULL;

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
			"Windows Async Client",
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
		sendMove();
		keyPress(hWnd);
	
	}

	return 0;
}

LRESULT CALLBACK WinProc(HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam)
{

	switch(msg)
	{
		case WM_CREATE:
		{

		  //Load Bitmap(Ball) At Start Instead Of Constant Redrawing
		  bmpSource = (HBITMAP)LoadImage(NULL, "ball.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
          hdcSource = CreateCompatibleDC(GetDC(0));
          SelectObject(hdcSource, bmpSource);
          
		  //Load Bitmap2(Face) At Start Instead Of Constant Redrawing
		  bmpSource2 = (HBITMAP)LoadImage(NULL, "autoBall.bmp", IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
          hdcSource2 = CreateCompatibleDC(GetDC(0));
          SelectObject(hdcSource2, bmpSource2);

		
			// Set Up Winsock
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

			nResult=WSAAsyncSelect(Socket,hWnd,WM_SOCKET,(FD_CLOSE|FD_READ));
			if(nResult)
			{
				MessageBox(hWnd,
					"WSAAsyncSelect failed",
					"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			//Resolve IP Address For Hostname
			struct hostent *host;
			if((host=gethostbyname(szServer))==NULL)
			{
				MessageBox(hWnd,
					"Unable to resolve host name",
					"Critical Error",
					MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}

			//Set Up Socket Address Structure
			SOCKADDR_IN SockAddr;
			SockAddr.sin_port=htons(nPort);
			SockAddr.sin_family=AF_INET;
			SockAddr.sin_addr.s_addr=*((unsigned long*)host->h_addr);

			connect(Socket,(LPSOCKADDR)(&SockAddr),sizeof(SockAddr));
		}
		break;

		case WM_PAINT:
			{
        //Draw Ball
	    hdcDestination = BeginPaint(hWnd, &ps);
	    BitBlt(hdcDestination, BmpX, BmpY, 500, 500, hdcSource, 0, 0, SRCCOPY);
		////Hide The Trail -- Drawing White Version Just After
		BitBlt(hdcDestination, BmpX-32.0f, BmpY, 500, 500, hdcSource, 0, 0, MERGEPAINT);	
		BitBlt(hdcDestination, BmpX+32.0f, BmpY, 500, 500, hdcSource, 0, 0, MERGEPAINT);
		////End The Painting
	    EndPaint(hWnd, &ps);

		//Updates Drawn Bitmaps  
		InvalidateRect(hWnd, NULL, FALSE);

	    //Draw Face
	    hdcDestination2 = BeginPaint(hWnd, &ps2);
	    BitBlt(hdcDestination2, FaceX, FaceY, 500, 500, hdcSource2, 0, 0, SRCCOPY);
		//End The Painting
	    EndPaint(hWnd, &ps);
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
			if(WSAGETSELECTERROR(lParam))
			{	
				MessageBox(hWnd,
					"Connection to server failed",
					"Error",
					MB_OK|MB_ICONERROR);
				SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				break;
			}
			switch(WSAGETSELECTEVENT(lParam))
			{
				case FD_READ:
				{

				}
				break;

				case FD_CLOSE:
				{
					//Connection Closed Message
					MessageBox(hWnd,
						"Server closed connection",
						"Connection closed!",
						MB_ICONINFORMATION|MB_OK);
					closesocket(Socket);
					SendMessage(hWnd,WM_DESTROY,NULL,NULL);
				}
				break;
			}
		} 
	}

	return DefWindowProc(hWnd,msg,wParam,lParam);
}

void keyPress(HWND hWnd)
{
	//Face Movement
	//If Inside Boundaries & Moving Up
	if (GetAsyncKeyState(VK_UP) && FaceY >= 0)
	{
		FaceY = FaceY - 0.06f;
		InvalidateRect(hWnd, NULL, FALSE);	
	}

	//If Inside Boundaries & Moving Down (Face)
	if (GetAsyncKeyState(VK_DOWN) && FaceY <= 370)
	{
		FaceY = FaceY + 0.06f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	//If Inside Boundaries & Moving Left (Face)
	if (GetAsyncKeyState(VK_LEFT) && FaceX >= 0)
	{
		FaceX = FaceX - 0.06f;
		InvalidateRect(hWnd, NULL, FALSE);
	}

	//If Inside Boundaries & Moving Right (Face)
	if (GetAsyncKeyState(VK_RIGHT) && FaceX <= 480)
	{
		FaceX = FaceX + 0.06f;
		InvalidateRect(hWnd, NULL, FALSE);
	}


	//Ball Calculations
	//If Inside Boundaries & Moving Right
	if(BmpX <= 580 && right == true)
	{
		BmpX = BmpX + 0.01f;	
		InvalidateRect(hWnd, NULL, FALSE);
		
		if(BmpX > 579)
			{
				right = false;
				left = true;
			}
	}

	//If Inside Boundaries & Moving Left
	if(BmpX >= 0 && left == true)
	{
		BmpX = BmpX - 0.01f;	
		InvalidateRect(hWnd, NULL, FALSE);

		if(BmpX < 0)
		{
			left = false;
			right = true;
		}
	}

	//Final Redraw
	InvalidateRect(hWnd, NULL, FALSE);
}


void sendMove()
{
	//Setup Buffer
	char Buffer2[PACKETSIZE], * pBuffer;
	pBuffer = Buffer2;
	
	//Face Information
	//Send Up To Spectator
	if(GetAsyncKeyState(VK_UP))
		{
			*pBuffer++ = 'W';
		}

	//Send Down To Spectator
	if (GetAsyncKeyState(VK_DOWN))
		{
			*pBuffer++ = 'S';	
		}

	//Send Left To Spectator
	if (GetAsyncKeyState(VK_LEFT))
		{
			*pBuffer++ = 'A';	
		}

	//Send Right To Spectator
	if (GetAsyncKeyState(VK_RIGHT))
		{
			*pBuffer++ = 'D';	
		}

	//Ball Information
	//Send Ball Right To Spectator
	if (right == true)
		{
			*pBuffer++ = 'K';	
		}
	//Send Ball Left To Spectator
	if(left == true)
		{
			*pBuffer++ = 'J';
		}


	//Send Information
		*pBuffer = 0;
		if(*Buffer2)
		{
			send(Socket, Buffer2, BUFFERSIZE ,0);
		}		
}