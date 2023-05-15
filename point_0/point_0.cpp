#include <gst/gst.h>
#include <pipeline_manager.h>


int main(int argc, char *argv[])
{
  GstElement *pipeline;             // Declare GStreamer pipeline
  GstBus *bus;                      // Declare GStreamer bus
  GMainLoop *loop;                  // Declare GMainLoop
  pipeline_manager* local_pipeline_manager = new pipeline_manager(argc, argv);

  /* Initialize GStreamer */
  gst_init(&argc, &argv);

  /* Create a new GMainLoop */
  loop = g_main_loop_new(NULL, FALSE);

  /* Set up the GStreamer pipeline */
  pipeline = local_pipeline_manager->setup_gst_pipeline(local_pipeline_manager->source_type, local_pipeline_manager->host, local_pipeline_manager->port);

  /* Start playing the pipeline */
  gst_element_set_state(pipeline, GST_STATE_PLAYING);
  g_print("Running...\n");

  /* Get the pipeline's bus */
  bus = gst_pipeline_get_bus(GST_PIPELINE(pipeline));

  /* Add a signal watch for the bus */
  gst_bus_add_signal_watch(bus);

  /* Connect the bus's "message" signal to the on_message callback function */
  //gst_bus_add_watch(bus, local_pipeline_manager->my_callback_loop, loop );
   g_signal_connect(G_OBJECT(bus), "message", G_CALLBACK(local_pipeline_manager->on_message), loop);
 /* g_signal_connect(loop, "run", G_CALLBACK(local_pipeline_manager->on_run_signal), NULL);
  g_signal_connect(loop, "run", G_CALLBACK(local_pipeline_manager->my_callback_loop), NULL);*/

  /* Unreference the bus */
  gst_object_unref(GST_OBJECT(bus));
  /* Run the GMainLoop */
  g_main_loop_run(loop);

  /* Stop the pipeline */
  gst_element_set_state(pipeline, GST_STATE_NULL);

  /* Unreference the pipeline */
  gst_object_unref(pipeline);

  return 0;
}
