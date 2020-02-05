// AcquireContinuos_Multicast.cpp
//
// This sample demonstrates how to open a Camera in multicast mode
// and how to receive a multicast stream.
//
// Two instances of this application must be started simultanously on different computers.
// The first application running on PC A acts as controller and has full access to the GigE camera.
// The second instance started on PC B opens the camera in stream mode.
// This instance is not able to control the camera, but is able to receive multicast streams.
// 
// To get this sample running, please start this application first on computer A in control mode.
// After computer A has started to receive frames, start the second instance of this 
// application in monitor mode on computer B 

#include <iostream> 
#include <stdio.h>
#include "Multicast_ConfigBaslerCamera.h"
#include "Multicast_Controlling.h"

// Namespace for using cout
using namespace std;
using namespace Multicast_Controlling;

static string WStringToString( const wstring& inString )
{
	string ret;
	unsigned int i;

	ret.resize( inString.length( ) );

	for( i=0; i<inString.length( ); i++ )
	{
		ret[i] = (char)inString[i];
	}

	return ret;
}

void Help(int argc, char* argv[])
{
	cout << "Usage:  " << argv[0] << " 0 1 0 1 1024 768 1 2 20 " << g_sFeaturesFath << " " << g_sIPConfigFile << "\"0,1,2,3\"" << " 10.20.200.103 " << DEFAULT_NUM_BUFFERS_TO_GRAB << " c d e f" << endl 
		<< "Argument 1: <multicasting_mode (0: controller; 1: monitor)> " << endl 
		<< "         2: <acquisition_mode (0: single acqu; 1: single trigger; 2: free run (with acquisition); 3: free run (start camera only, without acquisition); 4: monitoring; 5: monitoring mode for interpolation) 6: Interpolation. Press buton to save image.>  " << endl 
		<< "         3: <recording_mode (0: no recording; 1: recording)> "  << endl 
		<< "         4: <shared memory buffer enabled (0: no; 1: yes)> " << endl 
		<< "         5: <set_camera_width (-1: maximum width camera can provide)> " << endl 
		<< "         6: <set_camera_height (-1: maximum height camera can provide)> " << endl 
		<< "         7: <set_sleep_ms (0: maximum frame rate)> " << endl 
		<< "         8: <set_queue_size (2: default; larger size, higher latency/lower conflict)> " << endl 
		<< "         9: <set_frame_rate (15: default. Only works in free run mode)> " << endl 
		<< "         10: <set_pfs_path ( " << g_sFeaturesFath << ": default)> " << endl 
		<< "         11: <set_ipconfig_path ( " << g_sIPConfigFile << ": default)> " << endl 
		<< "         12: <select camera (such as \"0,2,4,5\")> " << endl
		<< "         13: <select baumer server address (port will be 70 as always) " << endl
		<< "         14: <number of buffers to grab for each camera(10 is the default) " << endl
		<< "         15: <disk_1 (recording disk (at tmp folder) for camera 1> " << endl 
		<< "         16: <disk_2 (recording disk (at tmp folder) for camera 2> " << endl 
		<< "         17: <disk_3 (recording disk (at tmp folder) for camera 3> " << endl 
		<< "         18: <disk_4 (recording disk (at tmp folder) for camera 4> " << endl 
		<< "         19: <disk_5 (recording disk (at tmp folder) for camera 5> " << endl 
		<< "         20: <disk_6 (recording disk (at tmp folder) for camera 6> " << endl 
		<< endl;
}

int main(int argc, char* argv[])
{
	if (argc == 1)
	{
		Help(argc, argv);
		exit(1);
	}

	CConfigBaslerCamera a_config;

	int n = 1;

	int a_multicasting_mode = (argc > n) ? atoi(argv[n]) : 0;  // 1
	a_config.SetMulticastingMode(a_multicasting_mode);

	n ++;

	int a_acquire_mode = (argc > n) ? atoi(argv[n]) : 1; // 2
	a_config.SetAcquireMode(a_acquire_mode);
	n ++;

	int a_recording = (argc > n) ? atoi(argv[n]) : 1; // 3
	a_config.SetRecording(a_recording);

	n ++;

	int a_shmenabled = (argc > n) ? atoi(argv[n]) : 1; // 4
	a_config.SetShmEnabled(a_shmenabled);

	n ++;

	int a_width = (argc > n) ? atoi(argv[n]) : CAMAERA_RESOLUTION_CHOOSE_MAX; // 5
	a_config.SetCameraWidth(a_width);

	n ++;

	int a_height = (argc > n) ? atoi(argv[n]) : CAMAERA_RESOLUTION_CHOOSE_MAX; // 6
	a_config.SetCameraHeight(a_height);

	n ++;

	int a_sleepms = (argc > n) ? atoi(argv[n]) : DEFAULT_SLEEP_TIME; // 7
	a_config.SetSleepMS(a_sleepms);

	n ++;

	int a_ququesize = (argc > n) ? atoi(argv[n]) : DEFAULT_QUEUE_SIZE; // 8
	a_config.SetQueueSize(a_ququesize);

	n ++;

	int a_fps = (argc > n) ? atoi(argv[n]) : DEFAULT_FRAME_RATE; // 9
	a_config.SetFrameRate(a_fps);

	n ++;

	if (argc > n)
	{
		a_config.SetFeaturesPath(argv[n]);//10
	}

	n ++;

	if (argc > n)
	{
		a_config.SetIPConfigFile(argv[n]);//11
	}

	n ++;



	if (argc > n)
	{
		a_config.SetSelectedCamera(argv[n]);//12
	}

	n ++;

	if (argc > n)
	{
		a_config.SetBaumerServerAddress(argv[n]);//13
	}

	n ++;


	int a_iBuffersToGrab = (argc > n) ? atoi(argv[n]) : DEFAULT_NUM_BUFFERS_TO_GRAB; // 14
	a_config.SetBuffersToGrab(a_iBuffersToGrab);
	cout << "number of buffers to grab...." << a_iBuffersToGrab << endl;


	n ++;
    
    // Tell the user to launch the multicast "controlling" application or the multicast "monitor" application
	cout << "Start multicast application in (c)ontrol or in (m)onitor mode? (c/m): " << endl << "  " << ((a_multicasting_mode == 0) ? "Control mode" : "Monitor mode") << endl;
    		
	Multicast_Controlling::CMulticastController a(a_config);

    printf("\n\nselecting:\n\n");

	if ( a_config.GetMulticastingMode() == MULTICAST_MONITOR)
	{
      printf("\n\n1:\n\n");
		return a.Acquire_Continuous_Monitor(); 
	}
    else
	{
      printf("\n\n2:\n\n");
		cout << "AcquisitionMode: " << endl;

		switch (a_config.GetAcquireMode()) 
		{
			case Mode_Acquire_SingleFrame:
			{
				cout << "  SingleFrame" << endl;
				return a.Acquire_SingleFrame();
			}
			case Mode_Acquire_Continuous_TriggerOn:
			{
				cout << "  Continuous with trigger on" << endl;
				return a.Acquire_MultipleFrames_TriggerOn();
			} 
			case Mode_Acquire_Continuous_FreeRun:
			{
				cout << "  Free Run" << endl;
				return a.Acquire_Continuous(); // Acquire_MultipleFrames();
			}
			case Mode_Trigger_Only: // we no longer 
			{
				cout << "  Trigger Only" << endl;
				return a.Acquire_MultipleFrames_TriggerOn_SendTrigger(FALSE); // Acquire_MultipleFrames();
			}
			case Mode_Acquire_Continuous_Monitor:
			{
				cout << "  Monitoring mode" << endl;
				return a.Acquire_Continuous_Monitor(); // Acquire_MultipleFrames();
			}
			case Mode_Acquire_Continuous_Monitor_Interpolation:
			{
				cout << "  Monitoring mode with interpolation" << endl;
				return a.Acquire_Continuous_Monitor(TRUE); // Acquire_MultipleFrames();
			}
            case Mode_Acquire_Continuous_Monitor_Interpolation_SaveImage:
			{
				cout << "  Monitoring mode with interpolation ( and save image by keypress)" << endl;
				return a.Acquire_Continuous_Monitor_IP_SaveImage(); // Acquire_MultipleFrames();
			}
			case Mode_Acquire_Record_Interpolation:		// read video from file and do interpolation
			{
				cout << " Interpolation based on Recording File" << endl;
				return a.Acquire_Record_IP();
			}

		}
	}
	printf("\n\n3:\n\n");
	system("pause");
	std::getchar();
	return 1;
}