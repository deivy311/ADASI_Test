# ADASI_Test
Requirements
The requirements to run the code are:

    Gstreamer 1.0
    OpenCV 3.4.2
    CMake 3.5.1
    cairo
    C++ 17
    VLC >3.0.6
Setup
In order to set up, compile and run the code you have to follow the next steps:

    Install Gstreamer 1.0
    Install OpenCV 3.4.2
    Install CMake 3.5.1
    Install VLC >3.0.6
    Clone the repository
        Clone the repository using the next link: 
            https://github.com/deivy311/ADASI_Test
        If you dont have the repository you can download the zip file, atached to the email named ADASI_Test.zip and unzip it.

    Compile the code
        Go to the folder where you clone the repository and run the next commands:
            mkdir build
            cd build
            cmake ../
            make
        You should see something like the next image.

    Run the code
        Go to the folder where you compile the code and run the executable file, for this a better explnation is in the next section.
            ./point_0
            ./point_1

Running the code
There are a total of 5 executables files, from de point_0 to point_5, each one of them has a different functionality. The parameters from Point 0 to Point 2_3 are the same, the only difference is the functionality of each one. And the parameters from Point 4 to Point 5 are the same.

For point_0, point_1 and point_2_3 the parameters are:

    Source type: Defines if we want to stream the camera or a file, by default is autovideosrc there are only two posible options: 

        videotestsrc or autovideosrc

        Examples:

        The input example for both of them are:

            Camera case:
            ./point_1 -s videotestsrc

            File case:
            ./point_0 -s autovideosrc
        
    Host: Defines the host to stream by defualt is "127.0.0.1" 

        Examples:

            Localhost case:
            sudo ./point_0 -h 127.0.0.1

    Port: Defines the port to stream by defualt is "5002" 

        Examples:

            Bydefault case:
            sudo ./point_0 -p 5002

For point_4, point_5 and point_2_3 the parameters are same as before but there a couple more:

    RTSP port: Defines the RTSP port to stream by defualt is "5001" 

        Examples:

            Bydefault case:
            sudo ./point_4 -p_rtsp 5001

    RTSP video file path: Defines the RTSP video file path to stream by defualt is "../Files/video_test_2.mp4", this videos are in the Files folder and also in the build/bin folder.

        Examples:

            Bydefault case:
            sudo ./point_5 -f ../Files/video_test_2.mp4

Results
    The executables point_0 and point_1, can show the results for the literal 1, it is a simple RTP pipeline where can stream a video file or a camera. 
    
    Case 1.1: RTP wit no overlay
        Given the next command input: 

            ./point_0 -s autovideosrc -h 127.0.0.1 -p 5002

        We can see the next results:

            <!-- ![alt text]( -->

        In order to see the results by yourself you have to open VLC and go to Media -> Open Network Stream and put the next address:

            tcp://127.0.0.1:5002
        The video related to this is in the Folder Results, named point_1_no_overlay.mp4 which means it only streams video without any overlay.

    Case 1.2: RTP with overlay, demo file
        Given the next command input: 

            ./point_1 -s videotestsrc -h 127.0.0.1 -p 5002
        
        We can see the next results:
            
                <!-- ![alt text]( -->
        In order to see the results by yourself you have to open VLC and go to Media -> Open Network Stream and put the next address:

            tcp://127.0.0.1:5002
        The video related to this is in the Folder Results, named point_1_overlay_file.mp4 which means it only streams video with overlay, in this case a red box.

    Case 1.3: RTP with overlay, camera
        Given the next command input: 

            ./point_1 -s autovideosrc -h 127.0.0.1 -p 5002

        We can see the next results:
            
                <!-- ![alt text]( -->
        In order to see the results by yourself you have to open VLC and go to Media -> Open Network Stream and put the next address:
            
            tcp://127.0.0.1:5002
        The video related to this is in the Folder Results, named point_1_overlay_camera.mp4 which means it only streams a cmera video with overlay, in this case a red box.

    The Executable point_2_3, can show the results for the literal 2 and 3, it is a RTP pipeline where can stream a video file or a camera and also can stream a video file or a camera with dinamica overlay and output the video in H264.
    
    Case 2.1: RTP with dinamic overlay in H264 format video output, demo file
        Given the next command input: 

            ./point_2_3 -s videotestsrc -h 127.0.0.1 -p 5002

        We can see the next results:
            
                <!-- ![alt text]( -->
        In order to see the results by yourself you have to open VLC and go to Media -> Open Network Stream and put the next address:
            
            tcp://127.0.0.1:5002
        The video related to this is in the Folder Results, named point_2_3_dinamic_overlay_file.mp4 which means it only streams a video file with dinamic overlay and output the video in H264 format.
    
    Case 2.2: RTP with dinamic overlay in H264 format video output, camera
        Given the next command input: 

            ./point_2_3 -s autovideosrc -h 127.0.0.1 -p 5002
        
        We can see the next results:
            
                <!-- ![alt text]( -->
        In order to see the results by yourself you have to open VLC and go to Media -> Open Network Stream and put the next address:
                
            tcp://127.0.0.1:5002
        The video related to this is in the Folder Results, named point_2_3_dinamic_overlay_camera.mp4 which means it only streams a camera video with dinamic overlay and output the camera video in H264 format.    
    
    The Executable point_4, can show the results for the literal 4, it is a RTSP pipeline over RTP where can stream a video file or a camera with dinamica overlay and output the video in H264.

    Case 4.1: RTSP with dinamic overlay in H264 format video output, demo file
        Given the next command input: 

            ./point_4 -s videotestsrc -h 127.0.0.1 -p 5002 -p_rtsp 5001 -f ../Files/video_test_2.mp4
        
        We can see the next results:
            
            RTP server

                <!-- ![alt text]( -->
            The video related to this is in the Folder Results, named point_4_RTP_dinamic_overlay_file.mp4 which means it streams a video file with dinamic overlay and output the video in H264 format.
            The VLC command to see the results is:

                tcp://127.0.0.1:5002

            RTSP server

                <!-- ![alt text]( -->
            The video related to this is in the Folder Results, named point_4_RTSP_dinamic_overlay_file.mp4 which means it streams a video file with dinamic overlay and output the video in H264 format.
            The VLC command to see the results is:

                rtsp://127.0.0.1:5001/test
    
    Case 4.2: RTSP with dinamic overlay in H264 format video output, camera
        Given the next command input: 

            ./point_4 -s autovideosrc

        We can see the next results:
                
                RTP server
    
                    <!-- ![alt text]( -->
                The video related to this is in the Folder Results, named point_4_RTP_dinamic_overlay_camera.mp4 which means it streams a camera video with dinamic overlay and output the video in H264 format.
                The VLC command to see the results is:
    
                tcp://127.0.0.1:5002

                RTSP server

                    <!-- ![alt text]( -->
                The video related to this is in the Folder Results, named point_4_RTSP_dinamic_overlay_camera.mp4 which means it streams a camera video with dinamic overlay and output the video in H264 format.
                The VLC command to see the results is:

                rtsp://127.0.0.1:5001/test

    Metrics:
    In the next image we can se the metrics comuted for the point_4 and point_5
    
        point_4
            <!-- ![alt text]( -->
        point_5
            <!-- ![alt text]( -->