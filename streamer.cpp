#include <gst/gst.h>
#include <gst/app/gstappsrc.h>
#include <librealsense2/rs.hpp>
#include <librealsense2/h/rs_types.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/imgproc.hpp>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <stdint.h>
#include <time.h>
#include <iostream>
#include <string>
#include <fstream>

const int WIDTH     = 640;
const int HEIGHT    = 480;
const int SIZE      = 640 * 480 * 3;
const int FRAME     = 30;


// camera always returns this for 1 / get_depth_scale()
const uint16_t ONE_METER = 999;

// Holds the RGB data coming from R200
struct rgb
{
    unsigned char r;
    unsigned char g;
    unsigned char b;
};


// Function descriptions with the definitions
static void     cb_need_data1( GstAppSrc *appsrc, guint unused_size, gpointer user_data );
static void     cb_need_data2( GstAppSrc *appsrc, guint unused_size, gpointer user_data );

static rs2::colorizer color_map;

// declare filteres
static rs2::threshold_filter thr_filter;   // Threshold - removes values outside recommended range
// rs2::colorizer color_filter;    // Colorize - convert from depth to RGB color
static rs2::disparity_transform depth_to_disparity(true);
static float min_depth = 0.29f;
static float max_depth = 10.0f;
static bool is_disparity = false;



//=======================================================================================
// The main entry into the application. DUH!
// arg[] will contain the IP address of the machine running QGroundControl
//=======================================================================================
gint main(gint argc, gchar *argv[])
{
    // App requires a valid IP address to where QGroundControl is running.
    // if( argc < 2 )
    // {
    //     printf( "Inform address as first parameter.\n" );
    //     exit( EXIT_FAILURE );
    // }
    // =================== Initialize Realsense ======================//
    rs2::pipeline pipe;
    rs2::config rs_cfg;

    // filter settings
    thr_filter.set_option(RS2_OPTION_MIN_DISTANCE, min_depth);
    thr_filter.set_option(RS2_OPTION_MAX_DISTANCE, max_depth);
    color_map.set_option(RS2_OPTION_HISTOGRAM_EQUALIZATION_ENABLED, 0);
    color_map.set_option(RS2_OPTION_COLOR_SCHEME, 9.0f);     // Hue colorization
    color_map.set_option(RS2_OPTION_MAX_DISTANCE, max_depth);
    color_map.set_option(RS2_OPTION_MIN_DISTANCE, min_depth);

    Use when streaming from pre recorded rosbag
    // rs_cfg.enable_device_from_file("/home/johnny/Downloads/d435i_sample_data/d435i_walk_around.bag");

    //Use when streaing from an intel realsense device
    rs_cfg.enable_stream(RS2_STREAM_DEPTH, WIDTH, HEIGHT, RS2_FORMAT_Z16, FRAME);
    rs_cfg.enable_stream(RS2_STREAM_COLOR, WIDTH, HEIGHT, RS2_FORMAT_RGB8, FRAME);
    pipe.start(rs_cfg);
    gpointer pPipe = (gpointer) &pipe;

    char        str_pipeline[ 400 ];    // Holds the pipeline
    GMainLoop   *loop       = NULL;     // Main app loop keeps app alive
    GstElement  *pipeline   = NULL;     // GStreamers pipeline for data flow
    GstElement  *appsrc1     = NULL;     // Used to inject buffers into a pipeline
    GstElement  *appsrc2     = NULL;     // Used to inject buffers into a pipeline

    GstCaps     *app_caps1   = NULL;     // Define the capabilities of the appsrc element
    GstCaps     *app_caps2   = NULL;     // Define the capabilities of the appsrc element

    GError      *error      = NULL;     // Holds error message if generated
    // GST_DEBUG=2;

    GstAppSrcCallbacks cbs1;             // Callback functions/signals for appsrc
    GstAppSrcCallbacks cbs2;             // Callback functions/signals for appsrc

    // Initialize GStreamer
    gst_init( &argc, &argv );

    loop = g_main_loop_new( NULL, FALSE );    // snprintf( str_pipeline, sizeof( str_pipeline ), "appsrc name=src1 ! queue ! videoconvert ! x264enc tune=zerolatency ! h264parse ! rtph264pay pt=96 ! queue udpsink host=127.0.0.1 port=5000");
    snprintf( str_pipeline, sizeof( str_pipeline ), "appsrc name=src1 ! queue ! videoconvert ! x264enc tune=zerolatency ! h264parse ! rtph264pay pt=96 ! udpsink host=127.0.0.1 port=5000 \
                                                     appsrc name=src2 ! queue ! videoconvert ! x264enc tune=zerolatency ! h264parse ! rtph264pay pt=96 ! udpsink host=127.0.0.1 port=5001");


    // Instruct GStreamer to construct the pipeline and get the beginning element appsrc.
    pipeline    = gst_parse_launch( str_pipeline, &error );
    if( !pipeline )
    {
        g_print( "Parse error: %s\n", error->message );
        return 1;
    }

    appsrc1      = gst_bin_get_by_name( GST_BIN( pipeline ), "src1" );
    appsrc2      = gst_bin_get_by_name( GST_BIN( pipeline ), "src2" );

    // Create a caps (capabilities) struct that gets feed into the appsrc structure.
    app_caps1 = gst_caps_new_simple( "video/x-raw",
                                    "format", G_TYPE_STRING, "BGR",
                                    "width", G_TYPE_INT, WIDTH, 
                                    "height", G_TYPE_INT, HEIGHT,
                                    "framerate", GST_TYPE_FRACTION, 30, 1,
                                    NULL );

    app_caps2 = gst_caps_new_simple( "video/x-raw",
                                    "format", G_TYPE_STRING, "BGR",
                                    "width", G_TYPE_INT, WIDTH, 
                                    "height", G_TYPE_INT, HEIGHT,
                                    "framerate", GST_TYPE_FRACTION, 30, 1,
                                    NULL );
    gst_app_src_set_caps( GST_APP_SRC( appsrc1 ), app_caps1 );
    gst_app_src_set_caps( GST_APP_SRC( appsrc2 ), app_caps2 );

    // Don't need it anymore, un ref it so the memory can be removed.
    gst_caps_unref( app_caps1 );
    gst_caps_unref( app_caps2 );


    // Set a few properties on the appsrc Element
    g_object_set( G_OBJECT( appsrc1 ), "is-live", TRUE, "format", GST_FORMAT_TIME, NULL );
    g_object_set( G_OBJECT( appsrc2 ), "is-live", TRUE, "format", GST_FORMAT_TIME, NULL );

    // play
    gst_element_set_state( pipeline, GST_STATE_PLAYING );


    // ================== Connect GST signals with RS2 stream ================//
    cbs1.need_data = cb_need_data1;
    cbs2.need_data = cb_need_data2;

    // Apply the callbacks to the appsrc Elemen / Connect the signals.
    // In other words, cb_need_data will constantly being called
    // to pull data from R200. Why? Because it needs data. =)
    gst_app_src_set_callbacks( GST_APP_SRC_CAST( appsrc1 ), &cbs1, pPipe, NULL );
    gst_app_src_set_callbacks( GST_APP_SRC_CAST( appsrc2 ), &cbs2, pPipe, NULL );


    // Launch the stream to keep the app running rather than falling out and existing
    g_main_loop_run( loop );

    // clean up
    gst_element_set_state( pipeline, GST_STATE_NULL );
    gst_object_unref( GST_OBJECT ( pipeline ) );
    gst_object_unref( GST_OBJECT ( appsrc1 ) );
    gst_object_unref( GST_OBJECT ( appsrc2 ) );

    gst_object_unref( GST_OBJECT ( app_caps1 ) );
    gst_object_unref( GST_OBJECT ( app_caps2 ) );

    g_main_loop_unref( loop );

    return 0;
}

static void cb_need_data1( GstAppSrc *appsrc, guint unused_size, gpointer user_data )
{
    static int img_name = 0;

    // std::cout << "called: " << img_name << " times"  << std::endl;
    const int depth_size      = 640 * 480 * 3;

    GstFlowReturn ret;
    rs2::pipeline* pipe = (rs2::pipeline* ) user_data;
    rs2::frameset rs_d435 = pipe->wait_for_frames();
    // rs2::align align_to_color(RS2_STREAM_COLOR);
    // auto aligned_frame = align_to_color.process(rs_d415);
    // rs2::frame depth = rs_d435.get_depth_frame().apply_filter(color_map);
    rs2::frame depth = rs_d435.get_depth_frame();
    // apply post processing filters
    depth = thr_filter.process(depth);
    depth = depth_to_disparity.process(depth);
    // if (!is_disparity) { depth = disparity_to_depth.process(depth); }
    depth = color_map.process(depth);
    // rs2::frame color = rs_d415.get_color_frame();
    // cv::Mat imRGB(cv::Size(WIDTH, HEIGHT), CV_8UC3, (void *) color.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat imD(cv::Size(WIDTH, HEIGHT), CV_8UC3, (void *) depth.get_data(), cv::Mat::AUTO_STEP);

    img_name += 1;
    std::cout << "called: " << img_name << " times"  << std::endl;

    std::string img_name_str = "local_images/" + std::to_string(img_name) + ".png";
    // cv::imwrite(img_name_str, imD);

    // std::cout << "IM RGB: TYPE-" << imRGB.type() << " SIZE-" << imRGB.rows*imRGB.cols*imRGB.channels() << std::endl;

    // Creating a new buffer to send to the gstreamer pipeline
    GstBuffer *buffer = gst_buffer_new_wrapped_full( ( GstMemoryFlags )0, (gpointer)imD.data, depth_size, 0, depth_size, NULL, NULL );

    // Push the buffer back out to GStreamer so it can send it down and out to wifi
    g_signal_emit_by_name( appsrc, "push-buffer", buffer, &ret );

}

static void cb_need_data2( GstAppSrc *appsrc, guint unused_size, gpointer user_data )
{
    static int img_name = 0;

    // std::cout << "called: " << img_name << " times"  << std::endl;
    const int depth_size      = 640 * 480 * 3;

    GstFlowReturn ret;
    rs2::pipeline* pipe = (rs2::pipeline* ) user_data;
    rs2::frameset rs_d435 = pipe->wait_for_frames();
    rs2::frame color = rs_d435.get_color_frame();
    // cv::Mat imRGB(cv::Size(WIDTH, HEIGHT), CV_8UC3, (void *) color.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat imColor(cv::Size(WIDTH, HEIGHT), CV_8UC3, (void *) color.get_data(), cv::Mat::AUTO_STEP);
    // cv::Mat imD(cv::Size(WIDTH, HEIGHT), CV_16UC1, (void *) depth.get_data(), cv::Mat::AUTO_STEP);
    img_name += 1;
    // std::cout << "called: " << img_name << " times"  << std::endl;
    GstBuffer *buffer = gst_buffer_new_wrapped_full( ( GstMemoryFlags )0, (gpointer)imColor.data, SIZE, 0, SIZE, NULL, NULL );

    // Push the buffer back out to GStreamer so it can send it down and out to wifi
    g_signal_emit_by_name( appsrc, "push-buffer", buffer, &ret );

}

