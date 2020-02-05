// Multicast_Montitoring.cpp

// This source file demonstrates how to open and receive a multicast stream in controlling mode
// 
// This application must be started BEFORE the monitor application is started 

// Include files to use the PYLON API
#include "Multicast_Controlling.h"
#include <process.h>  // begin thread

BeginMCCNameSpace


int CMulticastController::Acquire_Continuous (void)
{
    int nRetVal = 0;        // return value, if no error ocurred
	int i, j;

    std::vector<CGrabBuffer*> BufferList[MAX_NUM_OF_CAMERAS];   // List of used stream buffers

    outLog << "Starting continuous acquisition in controlling mode." << endl << endl;

    // Automagically call PylonInitialize and PylonTerminate to ensure that the pylon runtime
    // system is initialized during the lifetime of this object
    Pylon::PylonAutoInitTerm autoInitTerm;

    try
    {
        // Get the transport layer factory
        CTlFactory& TlFactory = CTlFactory::GetInstance();

        // Create the transport layer object needed to enumerate or
        // create a camera object of type Camera_t::DeviceClass()
        ITransportLayer *pTl = TlFactory.CreateTl(Camera_t::DeviceClass());

        // Exit the application if the specific transport layer is not available
        if (! pTl)
        {
            cerr << "Failed to create transport layer!" << endl;
            return 1;
        }

        // Get all attached cameras and exit the application if no camera is found
        DeviceInfoList_t devices;

		m_uiNumCameras= pTl->EnumerateDevices(devices);

        if (0 == m_uiNumCameras)
        {
            cerr << "No camera present!" << endl;
            return 1;
        }

		outLog << "Number of camera present: " << m_uiNumCameras << endl;

		if ( ! devices.empty() ) 
		{
			DeviceInfoList_t::const_iterator it;
			for ( it = devices.begin(), i = 0; it != devices.end(); ++it, ++i )
			{
				outLog << "camera " << i << " " << "of " << devices.size() << ": " << it->GetFriendlyName() << endl; 
			}
		}
		else
		{
			cerr << "No devices found!" << endl;
		}

		DeviceInfoList_t::const_iterator it;
		for ( it = devices.begin(), i = 0, j = 0; it != devices.end(); ++it, j ++ )
		{

			int id_camera = GetCameraID(it->GetFriendlyName());

			if (id_camera == INVALID_CAMERA_ID)
			{
				continue;
			}
#if HS_DEBUG
			outLog << "debug: camera friend name: " << it->GetFriendlyName().c_str() << endl;
			outLog << "debug: camera full name: " << it->GetFullName().c_str() << endl;
#endif
			if (!m_config.IsSelectedCamera(id_camera))
			{
				continue;
			}

			// Create the camera object of the first available camera.
			// The camera object is used to set and get all available
			// camera features.
			// m_pCameras[i] = new Camera_t(pTl->CreateDevice(devices[ i ]));
			m_pCameras[i] = new Camera_t(pTl->CreateDevice(devices[ j ]));   // !!!!!!!!!!! device is j

			// Open the camera in controlling mode, but not exclusive
			m_pCameras[i]->Open (Stream | Control);

			// Get the first stream grabber object of the selected camera
			m_pGrabbers[i] = new Camera_t::StreamGrabber_t(m_pCameras[i]->GetStreamGrabber(0));

			// Set transmission type to "multicast"...
			// In this case, the IP Address and the IP port should also be set.
			SetStreamGrabber(i, m_pGrabbers[i], it->GetFriendlyName());

			// Open the stream grabber
			m_pGrabbers[i]->Open();

			ConfigCamera(i);

#if HARDWARE_TRIGGERING // HARDWARE

			m_pCameras[i]->TriggerSelector.SetValue(TriggerSelector_FrameStart);  // hsun20100705: TriggerSelector_AcquisitionStart: will reduce frame rate into half
			m_pCameras[i]->TriggerMode.SetValue(TriggerMode_On);
			m_pCameras[i]->TriggerSource.SetValue(TriggerSource_Line1);
			// hsun: we can not set fps when triggering is on
			m_pCameras[i]->AcquisitionFrameRateEnable.SetValue(false);

#else // NO TRIGGERING

			// Set the camera to continuous frame mode
			m_pCameras[i]->TriggerSelector.SetValue(TriggerSelector_AcquisitionStart);
			m_pCameras[i]->TriggerMode.SetValue(TriggerMode_Off);

			printf("Note: trigger mode is set as off.\n");

			m_pCameras[i]->AcquisitionFrameRateEnable.SetValue(true);
			m_pCameras[i]->AcquisitionFrameRateAbs.SetValue(m_config.GetFrameRate());
#endif


			// m_pCameras[i]->TriggerMode.SetValue(TriggerMode_On);
			// m_pCameras[i]->TriggerSource.SetValue(TriggerSource_Software);
			// m_pCameras[i]->AcquisitionMode.SetValue(AcquisitionMode_Continuous);
			
			// hsun: set exposure by external application
			// m_pCameras[i]->ExposureMode.SetValue(ExposureMode_Timed);
			// m_pCameras[i]->ExposureTimeRaw.SetValue(10);
	        
			// We won't use image buffers greater than m_iBayerImgSize
			m_pGrabbers[i]->MaxBufferSize.SetValue(m_iBayerImgSize);

			// We won't queue more than m_config.GetBuffersToGrab() image buffers at a time
			m_pGrabbers[i]->MaxNumBuffer.SetValue(m_config.GetBuffersToGrab());

			// Allocate all resources for grabbing. Critical parameters like image
			// size now must not be changed until FinishGrab() is called.
			m_pGrabbers[i]->PrepareGrab();

			// Buffers used for grabbing must be registered at the stream grabber.
			// The registration returns a handle to be used for queuing the buffer.
			for (uint32_t i_img = 0; i_img < m_config.GetBuffersToGrab(); ++i_img)
			{
				CGrabBuffer *pGrabBuffer = new CGrabBuffer(m_iBayerImgSize);
				pGrabBuffer->SetBufferHandle(m_pGrabbers[i]->RegisterBuffer(
					pGrabBuffer->GetBufferPointer(), m_iBayerImgSize));

				// Put the grab buffer object into the buffer list
				BufferList[i].push_back(pGrabBuffer);
			}


			for (std::vector<CGrabBuffer*>::const_iterator x = BufferList[i].begin(); x != BufferList[i].end(); ++x)
			{
				// Put the buffer into the grab queue for grabbing
				m_pGrabbers[i]->QueueBuffer((*x)->GetBufferHandle(), NULL);
			}

			i ++;
		}

		// hsun: update camera number based on selected camera input
		m_uiNumCameras = i; 

        outLog << "Camera connected successful." << endl;

		// hsun: add shm and recording
		InitizeAcquisitionAndRecording( );

        outLog << "Start streaming until a key was pressed." << endl;
        
        // Let the camera acquire images continuously ( Acquisiton mode equals
        // Continuous! )
		for ( i =0 ; i < m_uiNumCameras; i ++ )
		{
			m_pCameras[i]->AcquisitionStart.Execute();
		}
        
		Beep(100, 1000);
		timeBeginPeriod(1);
		int a_iTimeBegin = timeGetTime();

        // Grab c_ImagesToGrab times
        m_frame = 0;
        while (!_kbhit())
        {
            ++ m_frame; 

			// hsun: shm and recording
			AcquireImageFromCameras();
				
			if (m_frame % 100 == 0)
			{
				int	a_iTimeEnd	 = timeGetTime();
				int a_iTimeElapsed = a_iTimeEnd - a_iTimeBegin;

				float a_fps = (float)m_frame * 1000.0f / (float)a_iTimeElapsed;

				printf(" ->fps = %f[%d:%.1f] : result fps = %f\n", a_fps, m_frame, a_iTimeElapsed/1000.0f,  m_pCameras[0]->ResultingFrameRateAbs.GetValue());
			}
			
        } // While

		printf("Warning: keyboard hit, exiting.....\n");

		int	a_iTimeEnd	 = timeGetTime();
		int a_iTimeElapsed = a_iTimeEnd - a_iTimeBegin;

		float a_fps = (float)m_frame * 1000.0f / (float)a_iTimeElapsed;

		timeEndPeriod(1);

		printf("------------->fps = %f[%d:%.1f]\n", a_fps, m_frame, a_iTimeElapsed/1000.0f);
	
		Beep(100, 1000);
		
		// Release shm and recording related thread and handles)
		ReleaseRecording();

		// for ( it = devices.begin(), i = 0; it != devices.end(); ++it, i ++ )
		for ( i =0 ; i < m_uiNumCameras; i ++ )
		{
			// Stop acquisition
			m_pCameras[i]->AcquisitionStop.Execute();
	        
			// Get the pending buffer back (You are not allowed to deregister
			// buffers when they are still queued)
			m_pGrabbers[i]->CancelGrab();

			// Get all buffers back
			for (GrabResult r; m_pGrabbers[i]->RetrieveResult(r););

			// Clean up
			// You must deregister the buffers before freeing the memory
			for (std::vector<CGrabBuffer*>::iterator iter = BufferList[i].begin(); iter != BufferList[i].end(); iter ++)
			{
				m_pGrabbers[i]->DeregisterBuffer((*iter)->GetBufferHandle());
				delete *iter;
				*iter = NULL;
			}

			// Free all resources used for grabbing
			m_pGrabbers[i]->FinishGrab();

			// Close stream grabber
			m_pGrabbers[i]->Close();

			// Close camera
			m_pCameras[i]->Close();

			delete m_pGrabbers[i];
			delete m_pCameras[i];

		}

    }
    catch (GenICam::GenericException &e)
    {
        // Error handling
        cerr << "An exception occurred!" << endl

        << e.GetDescription() << endl;
        nRetVal = 1;
    }

    // Avoid memory leak in case of an error
    for (std::vector<CGrabBuffer*>::iterator it = BufferList[i].begin(); it != BufferList[i].end(); it++)
    {
        if (NULL != *it)
        {
            delete *it;
            *it = NULL;
        }
    }

    // Quit the application
    return nRetVal;
}


EndMCCNameSpace