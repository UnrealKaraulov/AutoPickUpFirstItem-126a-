
#pragma region Headers
#define _WIN32_WINNT 0x0501 
#define WINVER 0x0501 
#define NTDDI_VERSION 0x05010000
//#define BOTDEBUG
#define WIN32_LEAN_AND_MEAN
#include <stdexcept>
#include <Windows.h>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <time.h>
#include <thread>  
#pragma endregion


int GameDll = 0;
int _W3XGlobalClass = 0;


// 1. Ждать нажатия ALT
// 2. Создать список предметов рядом с героем
// 3. Ждать пока не появится новый предмет
// 4. Поднять первый появившийся предмет который находится в радиусе 100 от героя



// 1. Найти героя игрока, выбранный юнит.
// 2. 


#define IsKeyPressed(CODE) (GetAsyncKeyState(CODE) & 0x8000) > 0



BOOL IsGame( ) // my offset + public
{
	return ( *( int* ) ( ( UINT32 ) GameDll + 0xBE6530 ) > 0 )/* && !IsLagScreen( )*/;
}

BOOL IsWindowActive( )
{
	return *( BOOL* ) ( GameDll + 0xB673EC );
}


void SetTlsForMe( )
{
	UINT32 Data = *( UINT32 * ) ( GameDll + 0xBB8978 );
	UINT32 TlsIndex = *( UINT32 * ) ( GameDll + 0xBB8628 );
	if ( TlsIndex )
	{
		UINT32 v5 = **( UINT32 ** ) ( *( UINT32 * ) ( *( UINT32 * ) ( GameDll + 0xBB896C ) + 4 * Data ) + 44 );
		if ( !v5 || !( *( LPVOID * ) ( v5 + 520 ) ) )
		{
			Sleep( 1000 );
			SetTlsForMe( );
			return;
		}
		TlsSetValue( TlsIndex, *( LPVOID * ) ( v5 + 520 ) );
	}
	else
	{
		Sleep( 1000 );
		SetTlsForMe( );
		return;
	}
}

UINT GetItemCountAndItemArray( int ** itemarray )
{
	int GlobalClassOffset = *( int* ) ( GameDll + 0xBE6350 );
	int ItemOffset1 = *( int* ) ( GlobalClassOffset + 0x3BC ) + 0x10;
	int ItemCount = *( int* ) ( ItemOffset1 + 0x604 );
	if ( ItemCount > 0 && ItemOffset1 > 0 )
	{
		*itemarray = ( int * ) *( int* ) ( ItemOffset1 + 0x608 );
		return ItemCount;
	}
	return 0;
}


UINT GetUnitCountAndUnitArray( int ** itemarray )
{
	int GlobalClassOffset = *( int* ) ( GameDll + 0xBE6350 );
	int UnitsOffset1 = *( int* ) ( GlobalClassOffset + 0x3BC );
	int UnitsCount = *( int* ) ( UnitsOffset1 + 0x604 );
	if ( UnitsCount > 0 && UnitsOffset1 > 0 )
	{
		*itemarray = ( int * ) *( int* ) ( UnitsOffset1 + 0x608 );
		return UnitsCount;
	}
	return 0;
}

void * GetGlobalPlayerData( )
{
	if ( *( int * ) ( 0xBE4238 + GameDll ) > 0 )
	{
		return ( void * ) *( int* ) ( 0xBE4238 + GameDll );
	}
	else
		return nullptr;
}

int GetPlayerByNumber( int number )
{
	if ( number == -1 )
		return -1;

	void * arg1 = GetGlobalPlayerData( );
	int result = -1;
	if ( arg1 != nullptr && arg1 )
	{
		result = ( int ) arg1 + ( number * 4 ) + 0x58;
		result = *( int* ) result;
	}
	return result;
}

int GetLocalPlayerNumber( )
{
	void * gldata = GetGlobalPlayerData( );
	if ( gldata != nullptr && gldata )
	{
		int playerslotaddr = ( int ) gldata + 0x28;
		return ( int ) *( short * ) ( playerslotaddr );
	}
	else
		return -1;
}


int GetLocalPlayer( )
{
	return GetPlayerByNumber( GetLocalPlayerNumber( ) );
}

UINT GetUnitOwnerSlot( int unitaddr )
{
	return *( int* ) ( unitaddr + 88 );
}


int GetSelectedOwnedUnit( )
{

	int plr = GetLocalPlayer( );
	if ( plr != -1 && plr )
	{

		int unitaddr = 0; // = *(int*)((*(int*)plr+0x34)+0x1e0);

		__asm
		{
			MOV EAX, plr;
			MOV ECX, DWORD PTR DS : [ EAX + 0x34 ];
			MOV EAX, DWORD PTR DS : [ ECX + 0x1E0 ];
			MOV unitaddr, EAX;
		}

		if ( unitaddr > 0 )
		{
			if ( GetUnitOwnerSlot( unitaddr ) == GetLocalPlayerNumber( ) )
			{
				return unitaddr;
			}
		}
	}
	return NULL;
}


void GetUnitLocation( int unitaddr, float * x, float * y )
{
	*x = *( float* ) ( unitaddr + 0x284 );
	*y = *( float* ) ( unitaddr + 0x288 );
}


void GetItemLocation( int itemaddr, float * x, float * y )
{
	int iteminfo = *( int * ) ( itemaddr + 0x28 );
	if ( iteminfo )
	{
		*x = *( float* ) ( iteminfo + 0x88 );
		*y = *( float* ) ( iteminfo + 0x8C );
	}
}

int GetUnitItemCODE( int unit_or_item_addr )
{
	return unit_or_item_addr;
}

float Distance( float dX0, float dY0, float dX1, float dY1 )
{
	return sqrt( ( dX1 - dX0 )*( dX1 - dX0 ) + ( dY1 - dY0 )*( dY1 - dY0 ) );
}

HANDLE ttt;


std::vector<int> startupclasses;
int ItemOrderAddr = 0;

#define ADDR(X,REG)\
	__asm MOV REG, DWORD PTR DS : [ X ] \
	__asm MOV REG, DWORD PTR DS : [ REG ]

void __stdcall ItemOrder( int itemaddr_a3, int orderid_a1 = 0xd0003, int unknown_a2 = 0, unsigned int unknown_a4 = 4, unsigned int unknown_a5 = 0 )
{

	__asm
	{

		PUSH unknown_a5;
		PUSH unknown_a4;
		ADDR( _W3XGlobalClass, ECX );
		MOV ECX, DWORD PTR DS : [ ECX + 0x1B4 ];
		PUSH itemaddr_a3;
		PUSH unknown_a2;
		PUSH orderid_a1;
		CALL ItemOrderAddr;
	}
}



DWORD __stdcall AutoPickupItemThread( LPVOID )
{
	int MaxTimer = 10;

	Sleep( 3000 );

	SetTlsForMe( );

	try
	{

		startupclasses.push_back( 1 );




		while ( true )
		{
			if ( IsKeyPressed( VK_F1 ) || IsKeyPressed( VK_NUMPAD1 ) )
			{
				if ( IsGame( ) && IsWindowActive( ) )
				{
					startupclasses.clear( );
					{
						int * itemsarray = 0;
						int itemCount = GetItemCountAndItemArray( &itemsarray );
						if ( itemCount > 0 && itemsarray )
						{
							for ( int i = 0; i < itemCount; i++ )
							{
								if ( itemsarray[ i ] > 0 && itemsarray[ i ] < INT_MAX )
								{
									bool needadd = true;

									for each( int itclass in startupclasses )
									{
										if ( itclass == GetUnitItemCODE( itemsarray[ i ] ) )
										{
											needadd = false;
											break;
										}
									}

									startupclasses.push_back( GetUnitItemCODE( itemsarray[ i ] ) );
								}
							}
						}
					}


					while ( IsKeyPressed( VK_F1 ) || IsKeyPressed( VK_NUMPAD1 ) )
					{
						if ( IsGame( ) )
						{
							{
								int * itemsarray = 0;
								int itemCount = GetItemCountAndItemArray( &itemsarray );
								if ( itemCount > 0 && itemsarray )
								{
									int currentunit = GetSelectedOwnedUnit( );
									
									if ( currentunit )
									{
										float currentunitx = 0.0f;
										float currentunity = 0.0f;
										GetUnitLocation( currentunit, &currentunitx, &currentunity );

										for ( int i = 0; i < itemCount; i++ )
										{
											int currentitem = itemsarray[ i ];

											if ( currentunit && currentitem )
											{
												if ( itemsarray[ i ] > 0 && itemsarray[ i ] < INT_MAX )
												{
													bool found = false;

													for each( int itclass in startupclasses )
													{
														if ( itclass == GetUnitItemCODE( itemsarray[ i ] ) )
														{
															float currentitemx = 1000.0f;
															float currentitemy = 1000.0f;

															GetItemLocation( currentitem, &currentitemx, &currentitemy );
															if ( Distance( currentunitx, currentunity, currentitemx, currentitemy ) < 400.0f )
															{
																found = true;
																break;
															}
														}
													}

													if ( !found )
													{
														float currentitemx = 1000.0f;
														float currentitemy = 1000.0f;

														GetItemLocation( currentitem, &currentitemx, &currentitemy );


														if ( Distance( currentunitx, currentunity, currentitemx, currentitemy ) < 200.0f )
														{
															ItemOrder( currentitem );
															Sleep( 75 );
															break;
														}

													}
												}
											}
										}
									}
								}
							}

							Sleep( 15 );
						}
						else
							break;
					}
				}
			}

			Sleep( 200 );
		}
	}
	catch ( ... )
	{
		_W3XGlobalClass = GameDll + 0xBE6350;
		ItemOrderAddr = GameDll + 0x3AE810;
		ttt = CreateThread( 0, 0, AutoPickupItemThread, 0, 0, 0 );

		return 0;
	}
	return 0;
}

BOOL WINAPI DllMain( HINSTANCE hDLL, UINT reason, LPVOID reserved )
{
	if ( reason == DLL_PROCESS_ATTACH )
	{
		GameDll = ( int ) GetModuleHandle( "Game.dll" );
		if ( !GameDll )
		{
			MessageBox( 0, "Error no game.dll found", "Error", MB_OK );
			return FALSE;
		}

		_W3XGlobalClass = GameDll + 0xBE6350;
		ItemOrderAddr = GameDll + 0x3AE810;
		ttt = CreateThread( 0, 0, AutoPickupItemThread, 0, 0, 0 );

	}
	else if ( reason == DLL_PROCESS_DETACH )
	{
		TerminateThread( ttt, 0 );
	}
	return TRUE;
}
