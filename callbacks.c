//callbacks.c... 
//	Callback routines for the GUI elements.
//
//	Not strictly callbacks actually. It didn't seem worthwhile to split
//	out some of the library functions the callbacks use.

#include "support.h"
#include "speakjet.h"
#include "serial.h"
#include "handles.h"
#include "dictionary.h"

#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>

#include <gtk/gtk.h>

void HandleCode(unsigned char code);
void HandleCodeWithValue(unsigned char code, char *valueholder);
void SetPitch(unsigned char pitch); 
int SingleOrDouble(unsigned char code);
void InsertText(char *texttoinsert);
int FetchValue(unsigned char code);
long int GetTimeinMs(void);
char *GetNextCommand(char *haystack,char *result);
void ViewData(char *outputstring, char *format);

extern void xlate_word(char *word, unsigned char *wordcodes);

#define DOUBLECLICK 2
#define SINGLECLICK 1

unsigned char lastcode=0;
unsigned char lastpitch=0;
long lastcode_time;

void on_button_settings_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_entry_set_text(GTK_ENTRY(entry_serialdev),portdevice);
	//combobox_baudrate
	
	gtk_dialog_run(dialog_settings);
}

void on_button_settingsok_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	const char *newportdevice;
	newportdevice=gtk_entry_get_text(GTK_ENTRY(entry_serialdev));
	if(newportdevice==NULL)
		return;
	if(portdevice!=NULL)
		free(portdevice);
	portdevice=malloc(strlen(newportdevice)+1);
	strcpy(portdevice,newportdevice);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_19200)))
		baudrate=19200;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_9600)))
		baudrate=9600;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_4800)))
		baudrate=4800;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_2400)))
		baudrate=2400;
	reopen_port();
	gtk_widget_hide(GTK_WIDGET(dialog_settings));
}

void on_button_settingstest_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	char *saveserialdev;
	int savebaud;
	const char *newportdevice;

	newportdevice=gtk_entry_get_text(GTK_ENTRY(entry_serialdev));


	savebaud=baudrate;
	saveserialdev=portdevice;

	if(newportdevice==NULL)
		return;

	portdevice=malloc(strlen(newportdevice)+1);
	strcpy(portdevice,newportdevice);
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_19200)))
		baudrate=19200;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_9600)))
		baudrate=9600;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_4800)))
		baudrate=4800;
	if(gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON (radiobutton_2400)))
		baudrate=2400;
	reopen_port();

	char testvox[6] = { 31, 192, 131, 8, 187, 191 };
	int t;
	for(t=0;testvox[t]!=0;t++)
	{
		write(serialfd,testvox+t,1);
		usleep(500); //speakjet freaks out without a 500us delay between chars
	}
	
	
	// restore the old settings, in case the user cancels the dialog...
	free(portdevice);
	portdevice=saveserialdev;
	baudrate=savebaud;
	reopen_port();
}

void on_button_settingscancel_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(dialog_settings));
}
void on_dialog_settings_close()
{
	gtk_widget_hide(GTK_WIDGET(dialog_settings));
}

void on_button_about_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_dialog_run(dialog_about);
}

void on_button_aboutclose_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(dialog_about));
}

void on_dialog_about_close()
{
	gtk_widget_hide(GTK_WIDGET(dialog_about));
}

void on_button_P0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P0_code);
}

void on_button_P1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P1_code);
}

void on_button_P2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P2_code);
}

void on_button_P3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P3_code);
}

void on_button_P5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P5_code);
}

void on_button_P4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P4_code);
}

void on_button_P6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(P6_code);
}

void on_button_delay_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(delay_code,"entry_delay");
}
void on_button_O1C_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1C_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2C_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2C_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3C_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3C_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1Cs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1Cs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2Cs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2Cs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3Cs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3Cs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1D_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1D_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2D_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2D_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3D_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3D_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1Ds_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1Ds_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2Ds_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2Ds_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3Ds_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3Ds_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1E_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1E_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2E_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2E_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3E_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3E_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1F_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1F_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2F_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2F_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3F_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3F_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1Fs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1Fs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2Fs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2Fs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3Fs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3Fs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1G_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1G_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2G_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2G_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3G_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3G_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1Gs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1Gs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2Gs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2Gs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3Gs_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3Gs_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1A_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1A_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2A_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2A_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3A_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3A_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1As_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1As_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2As_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2As_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3As_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3As_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O2B_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O2B_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O3B_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O3B_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_O1B_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//change the pitch to match this note
	SetPitch(O1B_pitch);
	HandleCodeWithValue(pitch_code,"entry_pitch");
}

void on_button_IY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IY_code);
}

void on_button_UW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(UW_code);
}

void on_button_IH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IH_code);
}

void on_button_UH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(UH_code);
}

void on_button_EY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EY_code);
}

void on_button_AX_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AX_code);
}

void on_button_OW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OW_code);
}

void on_button_EH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EH_code);
}

void on_button_OH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OH_code);
}

void on_button_AY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AY_code);
}

void on_button_UX_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(UX_code);
}

void on_button_AW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AW_code);
}

void on_button_OWIY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OWIY_code);
}

void on_button_EHLE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EHLE_code);
}

void on_button_IHWW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IHWW_code);
}

void on_button_OHIY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OHIY_code);
}

void on_button_OHIH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OHIH_code);
}

void on_button_AXUW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AXUW_code);
}

void on_button_OWWW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OWWW_code);
}

void on_button_EYIY_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EYIY_code);
}

void on_button_IYEH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IYEH_code);
}

void on_button_IYUW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IYUW_code);
}

void on_button_AYWW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AYWW_code);
}

void on_button_WW_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(WW_code);
}

void on_button_MM_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(MM_code);
}

void on_button_NGO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(NGO_code);
}

void on_button_NO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(NO_code);
}

void on_button_LO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(LO_code);
}

void on_button_LE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(LE_code);
}

void on_button_NE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(NE_code);
}

void on_button_NGE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(NGE_code);
}

void on_button_RR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(RR_code);
}

void on_button_OWRR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OWRR_code);
}

void on_button_EYRR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EYRR_code);
}

void on_button_AWRR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AWRR_code);
}

void on_button_IYRR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(IYRR_code);
}

void on_button_AXRR_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(AXRR_code);
}

void on_button_JH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(JH_code);
}

void on_button_VV_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(VV_code);
}

void on_button_DH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(DH_code);
}

void on_button_ZH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(ZH_code);
}

void on_button_ZZ_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(ZZ_code);
}

void on_button_BE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(BE_code);
}

void on_button_DO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(DO_code);
}

void on_button_EG_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EG_code);
}

void on_button_BO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(BO_code);
}

void on_button_EB_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EB_code);
}

void on_button_OB_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OB_code);
}

void on_button_DE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(DE_code);
}

void on_button_ED_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(ED_code);
}

void on_button_OD_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OD_code);
}

void on_button_GE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(GE_code);
}

void on_button_GO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(GO_code);
}

void on_button_OG_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OG_code);
}

void on_button_WH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(WH_code);
}

void on_button_HO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(HO_code);
}

void on_button_HE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(HE_code);
}

void on_button_SO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(SO_code);
}

void on_button_TH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(TH_code);
}

void on_button_FF_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(FF_code);
}

void on_button_SE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(SE_code);
}

void on_button_SH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(SH_code);
}

void on_button_CH_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(CH_code);
}

void on_button_OK_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(OK_code);
}

void on_button_TS_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(TS_code);
}

void on_button_PO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(PO_code);
}

void on_button_PE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(PE_code);
}

void on_button_TT_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(TT_code);
}

void on_button_TU_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(TU_code);
}

void on_button_KE_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(KE_code);
}

void on_button_KO_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(KO_code);
}

void on_button_EK_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(EK_code);
}

void on_button_M0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(M0_code);
}

void on_button_M1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(M1_code);
}

void on_button_M2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(M2_code);
}

void on_button_D11_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D11_code);
}

void on_button_D10_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D10_code);
}

void on_button_D9_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D9_code);
}

void on_button_D8_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D8_code);
}

void on_button_D7_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D7_code);
}

void on_button_D6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D6_code);
}

void on_button_D5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D5_code);
}

void on_button_D4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D4_code);
}

void on_button_D3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D3_code);
}

void on_button_D2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D2_code);
}

void on_button_D1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D1_code);
}

void on_button_D0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(D0_code);
}

void on_button_C9_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C9_code);
}

void on_button_C8_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C8_code);
}

void on_button_C7_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C7_code);
}

void on_button_C6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C6_code);
}

void on_button_C5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C5_code);
}

void on_button_C4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C4_code);
}

void on_button_C3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C3_code);
}

void on_button_C2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C2_code);
}

void on_button_C1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C1_code);
}

void on_button_C0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(C0_code);
}

void on_button_B9_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B9_code);
}

void on_button_B8_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B8_code);
}

void on_button_B7_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B7_code);
}

void on_button_B6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B6_code);
}

void on_button_B5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B5_code);
}

void on_button_B4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B4_code);
}

void on_button_B3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B3_code);
}

void on_button_B2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B2_code);
}

void on_button_B1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B1_code);
}

void on_button_B0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(B0_code);
}

void on_button_A9_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A9_code);
}

void on_button_A8_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A8_code);
}

void on_button_A7_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A7_code);
}

void on_button_A6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A6_code);
}

void on_button_A5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A5_code);
}

void on_button_A4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A4_code);
}

void on_button_A3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A3_code);
}

void on_button_A2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A2_code);
}

void on_button_A1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A1_code);
}

void on_button_A0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(A0_code);
}

void on_button_R9_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R9_code);
}

void on_button_R8_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R8_code);
}

void on_button_R7_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R7_code);
}

void on_button_R6_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R6_code);
}

void on_button_R5_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R5_code);
}

void on_button_R4_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R4_code);
}

void on_button_R3_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R3_code);
}

void on_button_R2_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R2_code);
}

void on_button_R1_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R1_code);
}

void on_button_R0_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(R0_code);
}

void on_button_volume_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(volume_code,"entry_volume");
}
void on_button_speed_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(speed_code,"entry_speed");
}
void on_button_pitch_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(pitch_code,"entry_pitch");
}
void on_button_bend_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(bend_code,"entry_bend");
}
void on_button_resetvolume_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_entry_set_text(GTK_ENTRY(entry_volume),"96");
	HandleCodeWithValue(volume_code,"entry_volume");
	lastcode=0; //make sure we don't ever register double-clicks
}

void on_button_resetspeed_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_entry_set_text(GTK_ENTRY(entry_speed),"114");
	HandleCodeWithValue(speed_code,"entry_speed");
	lastcode=0; //make sure we don't ever register double-clicks
}

void on_button_resetpitch_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_entry_set_text(GTK_ENTRY(entry_pitch),"88");
	HandleCodeWithValue(pitch_code,"entry_pitch");
	lastcode=0; //make sure we don't ever register double-clicks
}

void on_button_resetbend_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_entry_set_text(GTK_ENTRY(entry_bend),"5");
	HandleCodeWithValue(bend_code,"entry_bend");
	lastcode=0; //make sure we don't ever register double-clicks
}

void on_button_fast_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(fast_code);
}

void on_button_stress_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(stress_code);
}

void on_button_slow_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(slow_code);
}

void on_button_relax_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler
	HandleCode(relax_code);
}

void on_button_repeat_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//call the double/single click handler for buttons with companion values
	HandleCodeWithValue(repeat_code,"entry_repeat");
}

void on_button_speak_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	GtkTextBuffer *gtkbuffer;
	char *buffer;
	GtkTextIter iterstart,iterend;
	char *p1;
	char commandword[255];
	unsigned char code;
	int t;

	//** some mojo to pull all of the text out of the text-view widget
	//** and point *buffer at it...
	gtkbuffer=gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_saydata));
	gtk_text_buffer_get_bounds(gtkbuffer,&iterstart,&iterend);
	buffer=gtk_text_buffer_get_text(gtkbuffer,&iterstart,&iterend,FALSE);


	code=31; //reset the pitch, speed, etc. first
	write(serialfd,&code,1);
	usleep(500); //speakjet freaks out without a 500us delay between chars

	p1=buffer;
	while(*p1!=0)
	{
		p1=GetNextCommand(p1,commandword);
		if(commandword[0]!=0)
		{
			for(t=0;t<256;t++)
			{
				if(strcmp(codetable[t],commandword)==0)
					break;
			}
			if(t<256)
			{
				code=t;
				write(serialfd,&code,1);

				//printf("code: %d\n",code);
			}
			else
			{
				if(isdigit(commandword[1]))
				{
					code=atoi(commandword+1);
					write(serialfd,&code,1);
					//printf("code: %d\n",code);
				}
				//else we do nothing - it's an unrecognized cmd
			}
		}
	usleep(500); //speakjet freaks out without a 500us delay between chars
	}
}

void on_button_clear_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	//set the text buffer to an empty string...
	gtk_text_buffer_set_text(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_saydata)),"",-1);
	
}

void on_button_viewdatadec_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	char outputstring[16*1024];
	ViewData(outputstring, "%d");
	gtk_label_set_label(GTK_LABEL(label_datatype),"decimal");
	gtk_text_buffer_set_text(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_speakdata)),outputstring,-1);
	gtk_dialog_run(dialog_viewdata);

}

void on_button_viewdatahex_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	char outputstring[16*1024];
	ViewData(outputstring, "$%02x");
	
	gtk_label_set_label(GTK_LABEL(label_datatype),"hexadecimal");
	gtk_text_buffer_set_text(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_speakdata)),outputstring,-1);
	gtk_dialog_run(dialog_viewdata);

}

void on_button_viewdataclose_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	gtk_widget_hide(GTK_WIDGET(dialog_viewdata));
}

void on_dialog_viewdata_close()
{
	gtk_main_quit();
}

void on_button_search_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	const char *valuestring;
	char *p1,*p2;
	char *textbuffer;
	char *phonemes;
	int t;
	char word[100];

	// copy the text from the search entry into a buffer...
	valuestring=gtk_entry_get_text(GTK_ENTRY(entry_dictionary));
	textbuffer=calloc(strlen(valuestring)+1,1);
	strcpy(textbuffer,valuestring);

	// push the search to lower case words
	p1=textbuffer;
	while(*p1!=0)
	{
		*p1=tolower(*p1);
		p1++;
	}

	p1=textbuffer;
	
	p1=textbuffer; p2=p1;
	while(*p1!=0)
	{
		while( (*p1!=0)&&(!islower(*p1)) )
			p1++;
		p2=p1;
		while( (*p2!=0)&&(islower(*p2)) )
			p2++;
		if(p1==p2)
			return;
		strncpy(word,p1,p2-p1);
		word[p2-p1]=0;
		phonemes=LookUpWord(word);
		if(phonemes!=NULL)
		{
			InsertText(phonemes);
			InsertText("  \\P6  ");
		}
		else
		{
			//prepare to lookup the word using talk2me...
			//1. it expects the words in upper case...
			for(t=0;t<100;t++)
				{
					if(word[t]==0)
						break;
					word[t]=toupper(word[t]);
				}

			//create a zeroed array of uchars for results...
			unsigned char talk2mecodes[100];
			memset(talk2mecodes,0,100);
			xlate_word(word,talk2mecodes);
			for(t=0;t<100;t++)
			{
				if(talk2mecodes[t]==0)
					break;
				InsertText(codetable[talk2mecodes[t]]);
			}
			InsertText("  \\P6  ");
		}
		p1=p2;
	}
}

void on_button_reset_clicked(GtkToggleButton *togglebutton, gpointer user_data)
{
	char resetvox[6] = { '\\', '0', 'R', 'X', 31, 0 };
	int t;
	for(t=0;resetvox[t]!=0;t++)
	{
		write(serialfd,resetvox+t,1);
		usleep(500); //speakjet freaks out without a 500us delay between chars
	}
}

void HandleCode(unsigned char code)
{
	if(SingleOrDouble(code)==SINGLECLICK)
	{
		write(serialfd,&code,1);
		//printf("single-click - HandleCode - %d %s\n",code,codetable[code]);
	}
	else
	{
		if( (code==P0_code) || (code==P1_code) || (code==P2_code) ||
		    (code==P3_code) || (code==P4_code) || (code==P5_code) ||
		    (code==P6_code) || (code==delay_code))
		{
			InsertText("  ");
			InsertText(codetable[code]);
			InsertText("  ");
		}
		else
			InsertText(codetable[code]);
	}
}

void HandleCodeWithValue(unsigned char code, char *valueholder)
{
	int value;
	unsigned char cvalue;
	char valuestring[15];

	value=FetchValue(code);
	if(value<0)
		return;
	cvalue=value;
	if(SingleOrDouble(code)==SINGLECLICK)
	{
		write(serialfd,&code,1);
		usleep(500); //speakjet freaks out without a 500us delay between chars
		write(serialfd,&cvalue,1);
	}
	else
	{
		InsertText(codetable[code]);
		sprintf(valuestring,"\\%d ",cvalue);
		InsertText(valuestring);
	}

}

void SetPitch(unsigned char pitch)
{
	char newpitch[4];
	sprintf(newpitch,"%d",pitch);
	gtk_entry_set_text(GTK_ENTRY(entry_pitch),newpitch);
	if(lastpitch!=pitch)
		lastcode=0; //make sure we don't register double-clicks for different pitches
	lastpitch=pitch;
}


int SingleOrDouble(unsigned char code)
{
	long thistime;
	thistime=GetTimeinMs();
	if(lastcode==code)
	{
		if(thistime-lastcode_time<500)
		{
			lastcode=0; 
			return(DOUBLECLICK);
		}
		
	}
	lastcode=code;
	lastcode_time=thistime;
	return(SINGLECLICK);
}

int FetchValue(unsigned char code)
{
	const char *valuestring;
	int value;
	GtkWidget *entryWidget;
	int minvalue=0;
	int maxvalue=255;

	//find the text field that contains the value for this code...
	switch(code)
	{
		case delay_code :
			minvalue=1;
			maxvalue=255;
			entryWidget=entry_delay;
			break;
		case volume_code :
			minvalue=0;
			maxvalue=127;
			entryWidget=entry_volume;
			break;
		case speed_code :
			minvalue=0;
			maxvalue=127;
			entryWidget=entry_speed;
			break;
		case pitch_code :
			minvalue=0;
			maxvalue=255;
			entryWidget=entry_pitch;
			break;
		case bend_code :
			minvalue=0;
			maxvalue=15;
			entryWidget=entry_bend;
			break;
		case repeat_code :
			minvalue=1;
			maxvalue=255;
			entryWidget=entry_repeat;
			break;
		default :
			fprintf(stderr,"** ERR: FetchValue() run on unknown code:%d\n",code);
			return(-1);
	}
	valuestring=gtk_entry_get_text(GTK_ENTRY(entryWidget));
	value=atoi(valuestring);
	if((value<minvalue)||(value>maxvalue))
		return(-1);
	return(value);
}

long GetTimeinMs()
{
	struct timeval tv1;
	struct timezone tz1;
	gettimeofday(&tv1,&tz1);
	return((long)((tv1.tv_sec*1000)+(tv1.tv_usec/1000)));
}
void InsertText(char *texttoinsert)
{
	//insert the text into the textview_saydata text
	gtk_text_buffer_insert_interactive_at_cursor(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_saydata)),texttoinsert,-1,TRUE);
	//gtk_text_buffer_insert_interactive_at_cursor(gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_saydata))," ",-1,TRUE);
}

char *GetNextCommand(char *haystack,char *result)
{
	char *p1,*p2;

	p1=haystack;
	while((*p1!=0)&&(*p1!='\\'))
		p1++;
	if(*p1==0)
	{
		result[0]=0;
		return(p1);
	}
	p2=p1+1;
	while((*p2!=0)&&((isalpha(*p2))||(isdigit(*p2))))
		p2++;
	strncpy(result,p1,(p2-p1));
	result[p2-p1]=0; //ensure the command is null terminated
	return(p2);
}

void ViewData(char *outputstring, char *format)
{
	GtkTextBuffer *gtkbuffer;
	char *buffer;
	GtkTextIter iterstart,iterend;
	char *p1;
	char commandword[255];
	char tmpstr[8];
	unsigned char code;
	int t,outindex;

	//** some mojo to pull all of the text out of the text-view widget
	//** and point *buffer at it...
	gtkbuffer=gtk_text_view_get_buffer (GTK_TEXT_VIEW (textview_saydata));
	gtk_text_buffer_get_bounds(gtkbuffer,&iterstart,&iterend);
	buffer=gtk_text_buffer_get_text(gtkbuffer,&iterstart,&iterend,FALSE);

	outputstring[0]=0;
	outindex=0;
	

	p1=buffer;
	while(*p1!=0)
	{
		p1=GetNextCommand(p1,commandword);
		if(commandword[0]!=0)
		{
			for(t=0;t<256;t++)
			{
				if(strcmp(codetable[t],commandword)==0)
					break;
			}
			if(t<256)
			{
				code=t;
				sprintf(tmpstr,format,code); 
				if(outindex%8>0)
					strcat(outputstring,", ");
				strcat(outputstring,tmpstr);
				if(outindex%8==7)
					strcat(outputstring,"\n");	
				outindex++;
			}
			else
			{
				if(isdigit(commandword[1]))
				{
					code=atoi(commandword+1);
					sprintf(tmpstr,format,code); 
					if(outindex%8>0)
						strcat(outputstring,", ");
					strcat(outputstring,tmpstr);
					if(outindex%8==7)
						strcat(outputstring,"\n");	
					outindex++;
				}
				//else we do nothing - it's an unrecognized cmd
			}
		}
	}
	strcat(outputstring,"\n");	

}

/*

void talk2me(char *wordin,char *outputcodes)
{

	int outfd[2];
	int infd[2];

	char output[2048];

	int oldstdin, oldstdout;

	outputcodes[0]=0;

	pipe(outfd); // Where the parent is going to write to
	pipe(infd); // From where parent is going to read

	oldstdin = dup(0); // Save current stdin
	oldstdout = dup(1); // Save stdout

	close(0);
	close(1);

	dup2(outfd[0], 0); // Make the read end of outfd pipe as stdin
	dup2(infd[1],1); // Make the write end of infd as stdout

	if(!fork())
	{

		char *argv[]={"talk2me",0};
		close(outfd[0]); // Not required for the child
		close(outfd[1]);
		close(infd[0]);
		close(infd[1]);
		execv(argv[0],argv);

	}
	else
	{

		close(0); // Restore the original std fds of parent
		close(1);
		dup2(oldstdin, 0);
		dup2(oldstdout, 1);
		close(outfd[0]); // These are being used by the child
		close(infd[1]);
		write(outfd[1],wordin,strlen(wordin)); // Write to childs stdin
		close(outfd[1]);
		output[read(infd[0],output,100)] = 0; // Read from childs stdout
	}
	
}
*/
