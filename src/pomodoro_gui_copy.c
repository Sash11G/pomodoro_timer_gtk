#include <gtk/gtk.h>
#include <windows.h>

#define BLOCK_DURATION 300
#define ICON_PATH "C:/Users/JGome/pomodoro_project/assets/custom_icon.png"
// Struct to hold widgets
typedef struct {
    GtkWidget *label;
    GtkWidget *progress_bar;
    GtkWidget *spin_pomodoro;
} TimerWidgets;

// Struct to handle the BlockEndTime
typedef struct {
    time_t block_end_time;
} BlockData;


// Global variables for timer
static int seconds_left = 0;       // Remaining time in seconds
static int pomodoro_duration = 25; // Default duration in minutes
static gboolean timer_running = FALSE; // Tracks if a timer is active
static void start_blocking_period(int duration_in_seconds);
static GtkCssProvider *provider = NULL;
static GtkWidget *progress_bar = NULL;



void cleanup_resources(GtkWidget *widget, gpointer data) {
    (void)widget;
    (void)data;

    // Free dynamically allocated BlockData
    BlockData *block_data = (BlockData *)data;
    if (block_data) {
        free(block_data);
    }

    // Other cleanup...
    if (provider) {
        g_object_unref(provider);
        provider = NULL;
    }

    g_print("Resources cleaned up. Exiting application.\n");
    gtk_main_quit();
}




static gboolean animate_stripes(gpointer data) {
    GtkWidget *progress_bar = GTK_WIDGET(data);

    static int position = 0;

    if (!provider) return FALSE; // Ensure the provider exists

    char css[512];
    snprintf(css, sizeof(css),
        "progressbar trough progress{"
        "  min-height: 15px;"
        "  border-radius: 5px;"
        "  background-image: repeating-linear-gradient("
        "      -45deg,"
        "      #9FE2BF,"
        "      #9FE2BF 10px,"
        "      #89CFF0 10px,"
        "      #89CFF0 20px"
        "  );"
        "  background-size: 200%% 100%%;"
        "  background-position: %dpx 0;"
        "  background-color: rgba(255, 255, 255, 0.5);"
        "}",
        position);

    GtkCssProvider *provider = gtk_css_provider_new();
    gtk_css_provider_load_from_data(provider, css, -1, NULL);

    GtkStyleContext *context = gtk_widget_get_style_context(progress_bar);
    gtk_style_context_add_provider(context, GTK_STYLE_PROVIDER(provider),
                                    GTK_STYLE_PROVIDER_PRIORITY_USER);


    position = (position + 2) % 100;

    return TRUE; // Continue the animation
}

// Function to apply CSS to the widgets
void apply_css() {
    if (!provider) {
        provider = gtk_css_provider_new();

        gtk_css_provider_load_from_data(provider,
            ".main-window {"
            "  background-color: #D1D5DB;"
            "}"
            ".timer-label {"
            "  font-family: Consolas;"
            "  font-size: 82px;"
            "  font-weight: bold;"
            "  color: #000000;" /* Dark gray text */
            "}"
            ".progress-bar progress {"
            "  background-color: #007acc;" /* Blue fill for progress */
            "}"
            "progressbar {"
            "  min-height: 25px;"
            "  border-radius: 5px;"
            "}"
            "progressbar trough {"
            "  min-height: 15px;"
            "  border-radius: 5px;"
            "}"
            "progressbar trough progress {"
            "  min-height: 15px;"
            "  border-radius: 5px;"
            "  background-image: repeating-linear-gradient("
            "      -45deg,"
            "      #9FE2BF,"
            "      #9FE2BF 10px,"
            "      #89CFF0 10px,"
            "      #89CFF0 20px"
            "  );"
            "  background-size: 200% 100%;" /* Allow the animation to move */
            "  animation: move-stripes 2s linear infinite;" /* Smooth stripe animation */  
            "  background-color: rgba(255, 255, 255, 0.5);" /* Transparent overlay */
            "}"
            "@keyframes move-stripes {"
            "  from {"
            "    background-position: 0 0;"
            "  }"
            "  to {"
            "    background-position: 100% 0;"
            "  }"
            "}", 
            -1, NULL);

        GdkScreen *screen = gdk_screen_get_default();
        gtk_style_context_add_provider_for_screen(
            screen,
            GTK_STYLE_PROVIDER(provider),
            GTK_STYLE_PROVIDER_PRIORITY_APPLICATION
        );
    }

}

void notify_user(const char *message) {
    // Create a GTK message dialog
    GtkWidget *dialog = gtk_message_dialog_new(NULL,
        GTK_DIALOG_MODAL | GTK_DIALOG_DESTROY_WITH_PARENT,
        GTK_MESSAGE_INFO,          // Information icon
        GTK_BUTTONS_OK,            // OK button
        "%s", message);            // Message content

    // Set the title of the dialog
    gtk_window_set_title(GTK_WINDOW(dialog), "System Lockdown");

    // Run the dialog (blocks interaction with other windows, but not the main loop)
    gtk_dialog_run(GTK_DIALOG(dialog));

    // Destroy the dialog after user interaction
    gtk_widget_destroy(dialog);
}


// Callback for the blocking period
static gboolean block_user(gpointer data) {
    BlockData *block_data = (BlockData *)data;

    if (time(NULL) < block_data->block_end_time) {
        LockWorkStation(); // Lock the workstation
        return TRUE;       // Schedule another callback
    } else {
        printf("Block period has ended. You can log in now.\n");
        free(block_data); // Free allocated memory
        return FALSE;     // Stop calling this function
    }
}

static gboolean notify_and_lock(gpointer data) {
    (void)data;
    start_blocking_period(BLOCK_DURATION); // Start blocking after delay
    return FALSE; // Run only once
}




// Function to start the blocking period
static void start_blocking_period(int duration_in_seconds) {
    BlockData *block_data = malloc(sizeof(BlockData));
    if (!block_data) {
        fprintf(stderr, "Memory allocation failed\n");
        return;
    }
    block_data->block_end_time = time(NULL) + duration_in_seconds;
    g_timeout_add_seconds(2, block_user, block_data);

}



// Timer update function
static gboolean update_timer(gpointer data) {
    static int last_seconds_left = -1; // Initialize to ensure the first update

    TimerWidgets *widgets = (TimerWidgets *)data;
    GtkWidget *label = widgets->label;
    GtkWidget *progress_bar = widgets->progress_bar;

    if (seconds_left != last_seconds_left) {
        last_seconds_left = seconds_left; // Update the last recorded state

        // Update UI (timer label)
        int minutes = seconds_left / 60;
        int seconds = seconds_left % 60;

        char buffer[16];
        snprintf(buffer, sizeof(buffer), "%02d:%02d", minutes, seconds);
        gtk_label_set_text(GTK_LABEL(label), buffer);

        // Update progress bar
        gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar),
                                      1.0 - (seconds_left / (double)(pomodoro_duration * 60)));
    }

    if (seconds_left > 0) {
        seconds_left--;  // Decrement remaining time
        return TRUE;     // Continue the timer
    }

    // Timer complete
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 1.0);
    notify_user("The system will lock in 10 seconds.");
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(progress_bar), 0.0);
    g_timeout_add_seconds(10, notify_and_lock, NULL);

    timer_running = FALSE; // Reset the running flag
    return FALSE;          // Stop the timeout
}


// Start timer function
static void start_timer(GtkWidget *button, gpointer data) {
    (void)button; // Suppress unused parameter warning
    TimerWidgets *widgets = (TimerWidgets *)data;
    GtkWidget *spin_pomodoro = widgets->spin_pomodoro;

    if (timer_running) {
        // Timer is already running, prevent starting another
        return;
    }

    // Mark timer as running
    timer_running = TRUE;
    printf("timer running: %s\n", timer_running ? "TRUE" : "FALSE");


    // Get Pomodoro duration from spin button
    pomodoro_duration = gtk_spin_button_get_value_as_int(GTK_SPIN_BUTTON(spin_pomodoro));
    seconds_left = pomodoro_duration * 60; // Convert minutes to seconds

    // Reset progress bar
    gtk_progress_bar_set_fraction(GTK_PROGRESS_BAR(widgets->progress_bar), 0.0);

    // Start the timer
    g_timeout_add(1000, update_timer, widgets);
}

// Update timer label when spin button changes
static void update_timer_label(GtkSpinButton *spin_button, gpointer label) {
    int pomodoro_time = gtk_spin_button_get_value_as_int(spin_button);
    char buffer[16];
    snprintf(buffer, sizeof(buffer), "%02d:00", pomodoro_time);
    gtk_label_set_text(GTK_LABEL(label), buffer);
}

// Main function
int main(int argc, char *argv[]) {
    GtkWidget *window;
    GtkWidget *grid;
    GtkWidget *label_timer;
    GtkWidget *button_start;
    GtkWidget *spin_pomodoro;


    // Initialize GTK
    gtk_init(&argc, &argv);
    
    // Create the main window
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(window), "Pomodoro Timer");
    gtk_window_set_default_size(GTK_WINDOW(window), 250, 300);
    gtk_window_set_resizable(GTK_WINDOW(window), FALSE);
    g_signal_connect(window, "destroy", G_CALLBACK(cleanup_resources), NULL);

    // Set a custom icon for the window
    if (!gtk_window_set_icon_from_file(GTK_WINDOW(window), ICON_PATH, NULL)) {
        g_warning("Failed to set window icon.");
    }

    // Assign custom name to the window for CSS targeting
    // Add class to the window for CSS styling
    gtk_style_context_add_class(gtk_widget_get_style_context(window), "main-window");

    // Create a grid layout
    grid = gtk_grid_new();
    gtk_grid_set_row_spacing(GTK_GRID(grid), 15);
    gtk_grid_set_column_spacing(GTK_GRID(grid), 15);
    gtk_widget_set_halign(grid, GTK_ALIGN_FILL); // Fill horizontally
    gtk_widget_set_valign(grid, GTK_ALIGN_FILL); // Fill vertically
    gtk_widget_set_hexpand(grid, TRUE);         // Allow horizontal expansion
    gtk_widget_set_vexpand(grid, TRUE);         // Allow vertical expansion

    // Add margins to the grid
    gtk_widget_set_margin_top(grid, 20);
    gtk_widget_set_margin_bottom(grid, 20);
    gtk_widget_set_margin_start(grid, 20);
    gtk_widget_set_margin_end(grid, 20);

    // Add the grid to the window
    gtk_container_add(GTK_CONTAINER(window), grid);

    // Timer Label
    label_timer = gtk_label_new("25:00");
    gtk_style_context_add_class(gtk_widget_get_style_context(label_timer), "timer-label"); // Add class
    gtk_grid_attach(GTK_GRID(grid), label_timer, 0, 0, 2, 1);

    // Start Timer Button
    button_start = gtk_button_new_with_label("Start Timer!");
    gtk_widget_set_size_request(button_start, 100, 30);
    gtk_grid_attach(GTK_GRID(grid), button_start, 0, 1, 2, 1);

    // Pomodoro Duration Setting
    GtkWidget *label_pomodoro = gtk_label_new("Pomodoro:");
    gtk_grid_attach(GTK_GRID(grid), label_pomodoro, 0, 2, 1, 1);
    spin_pomodoro = gtk_spin_button_new_with_range(1, 
    60, 1);
    gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_pomodoro), 25); // Default: 25 mins
    gtk_grid_attach(GTK_GRID(grid), spin_pomodoro, 1, 2, 1, 1);

    // Connect the spin button to update the timer label
    g_signal_connect(spin_pomodoro, "value-changed", G_CALLBACK(update_timer_label), label_timer);

    // Progress Bar
    GtkWidget *label_progress = gtk_label_new("Progress:");
    gtk_grid_attach(GTK_GRID(grid), label_progress, 0, 3, 1, 1);
    progress_bar = gtk_progress_bar_new();
    gtk_style_context_add_class(gtk_widget_get_style_context(progress_bar), "progress-bar"); // Add class

    g_timeout_add(500, animate_stripes, progress_bar); // Update every 100ms


    //grid
    gtk_grid_attach(GTK_GRID(grid), progress_bar, 1, 3, 1, 1);

    // Pack widgets into the struct
    TimerWidgets widgets = {label_timer, progress_bar, spin_pomodoro};

    // Connect the Start Timer button
    g_signal_connect(button_start, "clicked", G_CALLBACK(start_timer), &widgets);

    // Apply the CSS
    apply_css();

    // Show all widgets
    gtk_widget_show_all(window);

    // Start the GTK main loop
    gtk_main();


    return 0;
}
