extern  int main(void);
extern  int DesignLabel(struct header_entry *dflt,struct option_entry *option_list,char *fname);
extern  int AddOption(char *option_str,char attr,int width,int data_tp,char *option,struct option_entry *option_list);
extern  int AddOptionEntry(struct option_entry *option_list,struct option_entry *entry);
extern  int AllocateEntry(struct label_entry *new_entry);
extern  int AllocateHeader(struct header_entry *new_header);
extern  int AppendConfiguration(struct header_entry *list_ptr,struct header_entry *new_config);
extern  int DeleteConfiguration(struct header_entry *list_ptr,char *name);
extern  int DeleteElement(struct header_entry *list_ptr,int row,int col);
extern  int EmptyEntry(struct label_entry *entry_ptr);
extern  int EmptyList(struct header_entry *header_ptr);
extern  int FillLabelWindow(struct header_entry *list_ptr,struct wnd label);
extern  int FillNewEntry(int row,int col,struct option_entry *option,int attr,int width,struct label_entry *new_entry);
extern  int GetLabelFile(struct header_entry *list_ptr,char *filename,struct option_entry *option_list);
extern  int InsertEntry(struct label_entry *list_ptr,struct label_entry *new_entry);
extern  int LocateHeader(struct header_entry *list_ptr,struct header_entry *cursor,char *name);
extern  int LocateListPosition(int row,int col,struct header_entry *list_ptr,struct label_entry *cursor,struct label_entry *trailer,int location_code);
extern  int LocateOption(struct label_entry *option_list,struct label_entry *cursor,int opt_number);
extern  int LocateOption2(struct option_entry *option_list,struct option_entry *cursor,char attb);
extern  int PrntError(int row,int col,char *message);
extern  int ResetOptionScroll(struct option_entry *option_list,struct option_entry *cursor);
extern  int ResetScrollConfiguration(struct header_entry *list_ptr,struct header_entry *cursor);
extern  int ResetScrollList(struct label_entry *row_ptr,struct label_entry *col_ptr,struct header_entry *list_ptr);
extern  int SaveLabelFile(struct header_entry *list_ptr,char *filename,int changed);
extern  int ScrollConfiguration(struct header_entry *cursor,int direction);
extern  int ScrollList(struct label_entry *row_ptr,struct label_entry *col_ptr,struct label_entry *trailer);
extern  int ScrollOptionList(struct option_entry *cursor,int direction);
/* int AddOptionMenu(struct UNNAMED *option_menu,char *item); */
extern  int CloseLabelWindow(struct wnd label,struct header_entry *header,int changed);
/* int CommandProcessor(int x,int y,int changed,struct wnd label,struct wnd cmenu_window,struct wnd omenu_window,struct UNNAMED *cmem_file,struct UNNAMED *omem_file,struct option_entry *option_list,struct header_entry *header); */
extern  int ConstructString(struct option_entry *option,char *strng);
/* int CreateCommandMenu(struct UNNAMED *com_menu); */
extern  int DefineHeaderForm(struct header_entry *label_header);
extern  int DefineLabelWindow(struct wnd label,struct header_entry *header);
extern  int DisplayEntry(struct label_entry *entry,struct wnd label);
extern  int GetHeaderInfo(struct header_entry *header);
extern  int DefineCommandWindow(struct wnd command_window);
extern  int DefineOptionWindow(struct wnd option_window);
extern  int DefineConfigWindow(struct wnd config_window);
extern  int LabelProcess(struct wnd label,struct header_entry *header,int changed);
extern  int Quser(char *question);
/* int SetCommandWindow(struct wnd command_window,struct UNNAMED *memory_file); */
/* int SetOptionWindow(struct wnd option_window,struct UNNAMED *memory_file); */
extern  int UpdateConfigWindow(struct header_entry *header,struct wnd cwindow);
