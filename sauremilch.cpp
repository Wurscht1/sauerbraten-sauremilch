#include <thread>
#include <chrono>
#include <ncurses.h>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <sys/mman.h>

int* shellAmmoPtr   = NULL;
int* bulletAmmoPtr  = NULL;
int* rocketsAmmoPtr = NULL;
int* rifleAmmoPtr   = NULL;
int* grenadeAmmoPtr = NULL;
int* pistolAmmoPtr  = NULL;
int* godModePtr     = NULL;
int* shootTimerPtr  = NULL;

float* xPosPtr = NULL;
float* yPosPtr = NULL;
float* zPosPtr = NULL;

float latestZPos = 0.0f;

bool hasPointers = false;
bool hasGodmode = false;
bool noGravity = false;
bool rapidFire = false;

pid_t pid = 6942;
uintptr_t startAddress = 0xDEADBEEF;
std::string moduleName = "linux_64_client";


// Function to get the start address of a module for a given process
uintptr_t GetModuleStartAddress(pid_t pid, const std::string& moduleName) 
{
    std::string line;
    std::ifstream mapsFile("/proc/" + std::to_string(pid) + "/maps");

    if (mapsFile.is_open()) {
        while (std::getline(mapsFile, line)) {
            if (line.find(moduleName) != std::string::npos) {
                std::istringstream iss(line);
                uintptr_t startAddress, endAddress;
                char hyphen;

                if (iss >> std::hex >> startAddress >> hyphen >> endAddress) {
                    return startAddress;
                }
            }
        }
        mapsFile.close();
    }
	
    return 0;
}

bool setUpID()
{
	pid = getpid();
	startAddress = GetModuleStartAddress(pid, moduleName);
				
	if (startAddress != 0)
	{
		wprintw(stdscr, "[Sauremilch]: Found Module base address: %p\n", startAddress);
		return true;
	}
	else
	{
		wprintw(stdscr, "[Sauremilch]: Couldn't find module in the process. Aborting.. :(\n");
		return false;
	}
}

int setUpPointers()
{
	unsigned long base_address = startAddress;
	unsigned long pointer1 = *(unsigned long*)(base_address + 5410952);
	unsigned long pointer2 = *(unsigned long*)(pointer1 + 88);
	unsigned long pointer3 = *(unsigned long*)(pointer2 + 480);
	unsigned long pointer4 = *(unsigned long*)(pointer3 + 440);
	unsigned long pointer5 = *(unsigned long*)(pointer4 + 584);
	unsigned long pointer6 = *(unsigned long*)(pointer5 + 60);
				
	unsigned long shellAmmo   = pointer6 + 408;
	unsigned long bulletAmmo  = pointer6 + 412;
	unsigned long rocketsAmmo = pointer6 + 416;
	unsigned long rifleAmmo   = pointer6 + 420;
	unsigned long grenadeAmmo = pointer6 + 424;
	unsigned long pistolAmmo  = pointer6 + 428;
	
	shellAmmoPtr   = reinterpret_cast<int*>(static_cast<uintptr_t>(shellAmmo));
	bulletAmmoPtr  = reinterpret_cast<int*>(static_cast<uintptr_t>(bulletAmmo));
	rocketsAmmoPtr = reinterpret_cast<int*>(static_cast<uintptr_t>(rocketsAmmo));
	rifleAmmoPtr   = reinterpret_cast<int*>(static_cast<uintptr_t>(rifleAmmo));
	grenadeAmmoPtr = reinterpret_cast<int*>(static_cast<uintptr_t>(grenadeAmmo));
	pistolAmmoPtr  = reinterpret_cast<int*>(static_cast<uintptr_t>(pistolAmmo));
	
	pointer1 = *(unsigned long*)(base_address + 5410952);
	pointer2 = *(unsigned long*)(pointer1 + 88);
	pointer3 = *(unsigned long*)(pointer2 + 360);
	pointer4 = *(unsigned long*)(pointer3 + 104);
	pointer5 = *(unsigned long*)(pointer4 + 472);
	pointer6 = *(unsigned long*)(pointer5 + 52);
	
	unsigned long xPos = pointer6 + 48;
	unsigned long yPos = pointer6 + 52;
	unsigned long zPos = pointer6 + 56;
	
	xPosPtr = reinterpret_cast<float*>(static_cast<uintptr_t>(xPos));
	yPosPtr = reinterpret_cast<float*>(static_cast<uintptr_t>(yPos));
	zPosPtr = reinterpret_cast<float*>(static_cast<uintptr_t>(zPos));
	
	unsigned long godMode = pointer6 + 464;
	godModePtr = reinterpret_cast<int*>(static_cast<uintptr_t>(godMode));
	
	unsigned long shootTimer = pointer6 + 400;
	shootTimerPtr = reinterpret_cast<int*>(static_cast<uintptr_t>(shootTimer));
	
	hasPointers = true;
	return 1;
}

int changeExecutableMemoryRegion(uintptr_t startRegion, int range, bool rw)
{
	long page_size = sysconf(_SC_PAGESIZE);
	
	uintptr_t page_aligned_address = startRegion & ~(page_size - 1);
	unsigned char* targetAddress = reinterpret_cast<unsigned char*>(page_aligned_address);
	
	if(rw)
	{
		if (mprotect(targetAddress, range, PROT_READ | PROT_WRITE) == -1) 
		{
			perror("mprotect");
			return 1;
		}
		wprintw(stdscr, "Memory modification complete.\n");
	}
	else
	{
		if (mprotect(targetAddress, range, PROT_READ | PROT_EXEC) == -1)
		{
			perror("mprotect");
			return 1;
		}
		wprintw(stdscr, "Memory modification complete.\n");
	}
	return 0;
}

int bytePatch(std::string function, bool isEnabled)
{
	if(function == "gravity")
	{
		uintptr_t base = startAddress + 0x149DD0;
		changeExecutableMemoryRegion(base, 3, true);
		unsigned char* memoryAddress = reinterpret_cast<unsigned char*>(base);
		
		if(isEnabled)
		{
			*(memoryAddress + 0x0) = 0x90;
			*(memoryAddress + 0x1) = 0x90;
			*(memoryAddress + 0x2) = 0x90;
		}
		else
		{
			*(memoryAddress + 0x0) = 0x89;
			*(memoryAddress + 0x1) = 0x43;
			*(memoryAddress + 0x2) = 0x38;
		}
		
		changeExecutableMemoryRegion(base, 3, false);
		
		return 0;
	}
	return 1;
}

void addAmmo()
{
	*shellAmmoPtr = 6942;
	*bulletAmmoPtr = 99999;
	*rocketsAmmoPtr = 6942;
	*rifleAmmoPtr = 6942;
	*grenadeAmmoPtr = 6942;
	*pistolAmmoPtr = 6942;
}

void move(std::string direction)
{
	if(direction == "forwards")
	{
		*xPosPtr += 50.0f;
	}
	else if (direction == "backwards")
	{
		*xPosPtr -= 50.0f;
	}
	else if (direction == "right")
	{
		*yPosPtr += 50.0f;
	}
	else if (direction == "left")
	{
		*yPosPtr -= 50.0f;
	}
	else if (direction == "up")
	{
		*zPosPtr += 50.0f;
		latestZPos += 50.0f;
	}
	else
	{
		*zPosPtr -= 50.0f;
		latestZPos -= 50.0f;
	}
}

void callback() 
{
	(void) initscr();
    (void) nonl();
    (void) cbreak();
    (void) noecho();

    nodelay(stdscr, TRUE);
	scrollok(stdscr, true);
	
	wprintw(stdscr, "[Sauremilch]: Loaded successfully now press any key to initialize :)\n");
	
	int ch;
	
	bool loop = true;
	
	if(setUpID())
	{
    while (loop) 
	{
        ch = getch();
        if (ch != ERR && std::islower(ch))
		{
			if(hasPointers)
			{
				switch(ch)
				{
					case 'x': loop = false; break;
			
					//infinite ammo
					case 'p': addAmmo(); wprintw(stdscr, "[Sauremilch]: Added ammo\n"); break;
				
					//clip forwards
					case 'w': move("forwards"); wprintw(stdscr, "[Sauremilch]: Forwards.. Current X Position: %f\n", *xPosPtr); break;
				
					//clip backwards
					case 's': move("backwards"); wprintw(stdscr, "[Sauremilch]: Backwards.. Current X Position: %f\n", *xPosPtr); break;
			
					//clip left
					case'a': move("left"); wprintw(stdscr, "[Sauremilch]: Left.. Current Y Position: %f\n", *yPosPtr); break;
				
					//clip right
					case 'd': move("right"); wprintw(stdscr, "[Sauremilch]: Right.. Current Y Position: %f\n", *yPosPtr); break;
				
					//clip up
					case 'q': move("up"); wprintw(stdscr, "[Sauremilch]: Up.. Current Z Position: %f\n", *zPosPtr); break;
			
					//clip down
					case 'e': move("down"); wprintw(stdscr, "[Sauremilch]: Down.. Current Z Position: %f\n", *zPosPtr); break;
				
					//enable/disable Gravity
					case 'f':
						noGravity = !noGravity;
						latestZPos = *zPosPtr;
						
						bytePatch("gravity", noGravity);
					
						wprintw(stdscr, "[Sauremilch]: No-Gravity: %b\n", noGravity);
						break;
				
					//enable/disable Godmode
					case 'g':
						hasGodmode = !hasGodmode;
						
						if(hasGodmode)
						{
							*godModePtr = 1111;
						}
						else
						{
							*godModePtr = 0;
						}
						wprintw(stdscr, "[Sauremilch]: God mode: %b\n", hasGodmode);
						break;
					
					case 'r':
						rapidFire = !rapidFire;
						wprintw(stdscr, "[Sauremilch]: Rapid Fire: %b\n", rapidFire);
						break;
				}
			}
			else
			{
				if(setUpPointers() == 1)
				{
					wprintw(stdscr, "[Sauremilch]: Initialized Successfully :)\n");
				}
				else
				{
					wprintw(stdscr, "[Sauremilch]: ERROR during initialization, sry :(\n");
				}
			}
            wrefresh(stdscr);
        }
		
		if(noGravity && hasPointers)
		{
			*zPosPtr = latestZPos;
		}
		
		if(rapidFire && hasPointers)
		{
			*shootTimerPtr = 0;
		}
		
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
	wprintw(stdscr, "[Sauremilch]: Exiting\n");
    endwin();
	}
}

__attribute__((constructor))
void start_callbacks()
{
    std::thread worker(callback);
    worker.detach();
}