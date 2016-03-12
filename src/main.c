#include <pebble.h>

static Window *s_main_window;
static Layer *s_canvas_layer, *s_background_layer;
static TextLayer *s_time_layer, *s_date_layer, *s_step_layer;

char s_health_text_one[8];  

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL); 
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
  
  // Copy date into buffer from tm structure
  static char date_buffer[16];
  strftime(date_buffer, sizeof(date_buffer), "%a %d %b", tick_time);

  // Show the date
  text_layer_set_text(s_date_layer, date_buffer);
 
  // Show the steps!
  HealthMetric metric = HealthMetricStepCount;
  time_t start = time_start_of_today();
  time_t end = time(NULL);

  // Check the metric has data available for today
  HealthServiceAccessibilityMask mask = health_service_metric_accessible(metric, 
    start, end);


  
  if(mask & HealthServiceAccessibilityMaskAvailable) {
    // Data is available!

    snprintf(s_health_text_one, sizeof(s_health_text_one), "%d", (int)health_service_sum_today(metric));
    text_layer_set_text(s_step_layer,  s_health_text_one );
  } else {
    // No data recorded yet today
    text_layer_set_text(s_step_layer, "none");
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
}

static void canvas_update_proc(Layer *layer, GContext *ctx) {
//   Custom drawing happens here!
  GRect bounds = layer_get_bounds(layer);
  GPoint center = GPoint(bounds.size.w / 2, bounds.size.h / 2);
  uint16_t radius = 60;
  
  graphics_context_set_fill_color(ctx, GColorRed);  
  // Draw the outline of a circle
  graphics_draw_circle(ctx, center, radius);
  
  // Fill a circle
  graphics_fill_circle(ctx, center, radius);
}

static void main_window_load(Window *window) {
  // Get information about the Window
  Layer *root_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(root_layer);

  window_set_background_color(window, GColorBlack);
  
//   s_bitmap_layer = bitmap_layer_create(bounds);
//   s_background_layer = layer_create(bounds);
//   layer_set_background_color(s_background_layer, GColorBlack);
//   layer_add_child(root_layer, s_background_layer);
  
   
  // Create canvas layer
  s_canvas_layer = layer_create(bounds);
  // Assign the custom drawing procedure
  layer_set_update_proc(s_canvas_layer, canvas_update_proc);
  // Add to Window
  layer_add_child(window_get_root_layer(window), s_canvas_layer);
  
  
  // Create date TextLayer
  s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(45, 90), bounds.size.w, 21));
  text_layer_set_text_color(s_date_layer, GColorBlack);
  text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_font(s_date_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD ));
  text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);

  // Add to Window
  layer_add_child(root_layer, text_layer_get_layer(s_date_layer));
  
  
  // Create the TextLayer with specific bounds
  s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(65, 25), bounds.size.w, 50));

  // Improve the layout to be more like a watchface
  text_layer_set_background_color(s_time_layer, GColorClear);
  text_layer_set_text_color(s_time_layer, GColorBlack);
  text_layer_set_text(s_time_layer, "00:00");
  text_layer_set_font(s_time_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_MEDIUM_NUMBERS  ));
  text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);

  // Add it as a child layer to the Window's root layer
  layer_add_child(root_layer, text_layer_get_layer(s_time_layer));

  // Create Steps TextLayer
  s_step_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(115, 90), bounds.size.w, 50));
  text_layer_set_text_color(s_step_layer, GColorBlack);
  text_layer_set_background_color(s_step_layer, GColorClear);
  text_layer_set_font(s_step_layer, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD ));
  text_layer_set_text_alignment(s_step_layer, GTextAlignmentCenter);
  text_layer_set_text(s_step_layer, "Steps");
  
  // Add to Window
  layer_add_child(root_layer, text_layer_get_layer(s_step_layer));
}


static void main_window_unload(Window *window) {
  // Destroy TextLayer
  text_layer_destroy(s_time_layer);
}


static void init() {
  // Create main Window element and assign to pointer
  s_main_window = window_create();

  // Set handlers to manage the elements inside the Window
  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  // Show the Window on the watch, with animated=true
  window_stack_push(s_main_window, true);

  // Make sure the time is displayed from the start
  update_time();

  // Register with TickTimerService
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
}

static void deinit() {
  // Destroy Window
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}