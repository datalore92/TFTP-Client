#ifndef TFTP_GUI_H
#define TFTP_GUI_H

#include <gtk/gtk.h>

/**
 * Initialize and run the TFTP client GUI
 * 
 * @param argc Command line argument count
 * @param argv Command line arguments
 * @return Application exit status
 */
int run_tftp_gui(int argc, char *argv[]);

#endif // TFTP_GUI_H
