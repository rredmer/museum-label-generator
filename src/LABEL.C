/*
**  LABEL.C
**
**  GENERIC LABEL DESIGN AND PRINT ROUTINE FOR C PROGRAMMING
**
**  DESIGNED AND PROGRAMMED BY: RONALD D. REDMER, 1988
**
*/

#define WN_DEBUG
#define LINT_ARGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <wfd.h>
#include <wfd_glob.h>


typedef struct header_entry
      {
       char name[10];
       int rows;
       int cols;
       int p_rows;
       int p_cols;
       char use[28];
       struct header_entry *next_header;
       struct header_entry *last_header;
      };


typedef struct label_entry
      {
       int row;
       int col;
       char code;
       int wdth;
       char txt[50];
       struct option_entry *option;
       struct label_entry *next_row;
       struct label_entry *next_col;
      };

struct option_entry
      {
       char code;
       int width;
       char optstr[20];
       int dttp;
       struct option_entry *next_option;
       struct option_entry *last_option;
       char *option;
      };


int main()
{
  char fname[10];
  struct header_entry *list;
  struct option_list *options;

  init_wfd();

  list = (struct header_entry *)NULL;
  strcpy(fname,"temp.fmt"); 
  DesignLabel(list, options, fname);
  return(0);
}






/* Procedure DesignLabel:  This procedure carries out the main definition  */
/* of windows, forms, and memory files.  This could be considered the main */
/* line.  						     */

/***************************************************************************/
/*							     */
/* Variable Dictionary: DesignLabel.				     */
/*							     */
/* Parameters: default, option_list, fname.			     */
/*							     */
/* default:         ( header ptr ) to the default label format to be used  */
/*                   when printing.  It also serves as a floating listptr. */
/*							     */
/* option_list:     ( option ptr ) to the beginning of an EXISTING option  */
/*                   list.  Use AddOption procedure to define options.     */
/*							     */
/* fname:           ( char[] ) string containing the file name which will  */
/*                   be used in disk I/O.                		     */
/*							     */
/* Locals: Hcursor, commandwn, labelwn, configwn, optionwn, header_form    */
/*         header_field, option_mfile, command_mfile, changed, saved.      */
/*							     */
/* Hcursor:         ( header ptr ) used to scoll the configuration list in */
/*                   order not change the DEFAULT ptr unnecessarily.       */
/*                                                                         */
/* 							     */
/* commandwn:       ( window ) in which all of the available commands are  */
/*                   to be placed and menuing to be performed.             */
/*       							     */
/* labelwn:	( window ) in which all label entry information will   */
/*                   be displayed.                                         */
/*  							     */
/* configwn:        ( window ) used to scroll from one configuration to    */
/*                   another.  Will contain Last, Current, and Next label  */
/*                   configuration.                                        */
/* 							     */
/* optionwn:	( window ) in which all option strings will be         */
/*                   displayed and option menuing will be done.            */
/*							     */
/* header_form:     ( form ) which will process all header information     */
/*                   including labal name, number of rows ..etc.           */
/*							     */
/* header_field:    ( field ptr ) which is used to define fields on the    */
/*     		 header_form.				     */
/*							     */
/* option_mfile:    ( memory file ptr ) to the memory file containing all  */
/*                   option strings and codes.    		     */
/*							     */
/* command_mfile:   ( memory file ptr ) to the memory file containing all  */
/*                   command strings.                                      */
/*							     */
/* changed:         ( int ) boolean identifier which signals if label      */
/*                   changes have been reconfigured.		                  */
/*                                                                         */
/* saved:		( int ) boolean identifier which signals if label           */
/*            	 changes have been saved to disk.                           */
/*	                                                                        */
/***************************************************************************/
int DesignLabel(dflt, option_list, fname)
struct header_entry *dflt;
struct option_entry *option_list;
char fname[];
{
struct header_entry *Hcursor;
WINDOW commandwn, labelwn, configwn, optionwn;
DFORMPTR header_form;
DFIELDPTR header_field;
MFILEPTR option_mfile, command_mfile;
int changed, saved = 0;
option_mfile = mf_def(NULLP, 50, 28);
command_mfile = mf_def(NULLP, 8, 28);

if (EmptyList(dflt))
	{
	GetLabelFile (dflt, fname, option_list);
	}
if (EmptyList(dflt))
	{
	PrntError (10, 10, "Format file empty, new label mode assumed");
	AllocateHeader (dflt);
	GetHeaderInfo (dflt);
	}

Hcursor = dflt;
DefineLabelWindow (Hcursor, labelwn);
DefineOptionWindow (optionwn);
DefineConfigWindow (configwn);
DefineCommandWindow (commandwn);

set_wn (&labelwn);

CreateCommandMenu (command_mfile);
SetCommandWindow (commandwn, command_mfile);
SetOptionWindow (optionwn, option_mfile);

LabelProcess (labelwn, Hcursor, changed); 
if (isset_wn (&labelwn))
	{
	unset_wn (&labelwn);
	}
unset_wn (&commandwn);
unset_wn (&optionwn);
unset_wn (&configwn);
if (!saved)
	{
	if (Quser ("Save the changed format list to disk?"))
		{
		SaveLabelFile (Hcursor, fname, changed);
		}
	}
return(0);
}

/********************************************************************/
/* Procedure AddOption: Used by programmer to build a doubly linked */
/* option list from which the user will choose label entries.       */
/*                                                                  */
/* Variable Dictionary: AddOption                                   */
/*                                                                  */
/*                                                                  */
/* Option_str:      ( =< char[20] ) character array which holds the */
/*                  message to be displayed in the options menu.    */
/*                                                                  */
/* Attr:            ( char[1] ) character used to represent option  */
/*                  presence on label window.  It will appear in    */
/*                  inverse video and repeated WIDTH times.         */
/*                                                                  */
/* Width:           ( int ) value used to specify maximum field     */
/*                  width of OPTION.  Enables write protect fields  */
/*                  on label window so no entries overlap.          */
/*                                                                  */
/* Data_tp:         ( char[] ) string which specifies data type of  */
/*                  OPTION.  Used when retrieving data from memory. */
/*                                                                  */
/* Option:          ( char* ) ptr to location of OPTION in memory.  */
/*                                                                  */
/* Option_list:     ( based ) list ptr of options list to be        */
/*                  appended.  Included so multiple option list     */
/*                  may be made in the future.                      */
/*                                                                  */
/********************************************************************/
int AddOption (option_str, attr, width, data_tp, option, option_list)
char option_str[];
char attr;
int width;
int data_tp;
char *option;
struct option_entry *option_list;
{
struct option_entry *new_option;	                               /* temporary */
                                                                /* which is used to allocate */
                                                                /* new option entires.       */
new_option = (struct option_entry *) 
     malloc(sizeof (struct option_entry));                      /* allocate new option entry */
new_option -> next_option = (struct option_entry * ) NULL;      /* and initialize all fields */
new_option -> last_option = (struct option_entry * ) NULL;      /* including null pointers   */

new_option -> code = attr;                                      /* and parameters passed.    */   
strcpy (new_option -> optstr, option_str);
new_option -> width = width;
new_option -> option = option;
new_option -> dttp = data_tp;
AddOptionEntry (option_list, new_option);
return(0);
}

	 
	 

/* Procedure AddOptionEntry: This procedure is used by AddOption to  */
/* append Entry on to the existing options list.                     */

/*********************************************************************/
/*                                                                   */
/* Variable Dictionary: AddOptionEntry                               */
/*                                                                   */
/*                                                                   */
/* Globals: num_opts.                                                */
/*                                                                   */
/* Num_opts:        ( int ) keeps track of number of options in the  */
/*                  OPTION_LIST.                                     */   
/*                                                                   */
/* Parameters: option_list, entry.                                   */
/*                                                                   */
/* Option_list:     ( option ptr ) to option list to be appended to. */
/*                                                                   */
/* Entry:           ( option ptr ) to record to be appended onto     */
/*                   OPTION_LIST.                                    */
/*                                                                   */
/* Locals: done, cursor.                                             */
/*                                                                   */
/* Done:            ( int ) used as boolean loop control.            */
/*                                                                   */
/* Cursor:          ( option ptr ) used to scroll OPTION_LIST.       */
/*                                                                   */
/*********************************************************************/
int AddOptionEntry (option_list, entry)
struct option_entry *option_list, *entry;
{
int num_opts=0;
int done = 1;
struct option_entry *cursor;

ResetOptionScroll (option_list, cursor);			      /* set cursor	to beginning of Option_list. */
if (cursor == (struct option_entry *) NULL)           /* if cursor is at the end of Option_list  */
	{
	num_opts = 1;
	entry -> dttp = 1;	 /*** ??????????????????????? *****/
	option_list = entry;
	}               					      /* then tack entry onto Option_list.       */
else while (!done)
	{							      /* else do */
	if (cursor -> next_option == (struct option_entry * ) NULL) 
		{
		entry -> dttp = cursor -> dttp + 1;
		num_opts = entry -> dttp;
		entry -> last_option = cursor;			      /* if cursor at the end of the list then	  */
		cursor -> next_option = entry;			      /* append and return. */
		done = 1;
		}
	else if (entry -> code == cursor -> code)
		{
		cursor -> option = entry -> option;
		done = 1;
		}
	else
		ScrollOptionList (cursor, 1);			      /* else increment cursor and repeat.          */
	}
return(0);
}  /*add option*/

	 
	 
	 
AllocateEntry (new_entry)

/* Procedure AllocateEntry:  Allocates a new label entry record.     */
/* Further documentation not necessary.                              */

struct label_entry *new_entry;

    {
      new_entry = (struct label_entry * ) malloc (sizeof (struct label_entry));
      new_entry -> next_col = (struct label_entry * ) NULL;
      new_entry -> next_row = (struct label_entry * ) NULL;
    }

	 
	 
int AllocateHeader (new_header)

/* Procedure AllocateHeader:  Allocates a new header record.      	*/
/* Further documentation not necessary.                               */

struct header_entry *new_header;					
    {
      new_header = (struct header_entry * ) malloc (sizeof (struct header_entry));
      new_header -> next_header = (struct header_entry * ) NULL;               /* initialize pointers to NULL */
      new_header -> last_header = (struct header_entry * ) NULL;
 /***** ???????     new_header -> next_row = (struct label_entry * ) NULL; ????? *****/
    }



AppendConfiguration (list_ptr, new_config)

/* Procedure AppendConfiguration: Appends a new header record to the */
/* given label entry list.                                           */

/*********************************************************************/
/*                                                                   */
/* Variable Dictionary: AppendConfiguration                          */
/*                                                                   */
/*                                                                   */
/* Globals: none.                                                    */
/*                                                                   */
/* Parameters: list_ptr, new_config.                                 */
/*                                                                   */
/* List_ptr:        ( header ptr ) to the list onto which NEW_CONFIG */
/*                   is to be appended.  This is included to enable  */
/*                   multiple lists of configutation lists for       */
/*                   future enhancements.                            */
/*                                                                   */
/* New_config:      ( header ptr ) to the new configuration to be    */
/*                  appended.                                        */
/*                                                                   */
/* Locals: done, cursor.                                             */
/*                                                                   */
/* Done:            ( int ) used as boolean loop control.            */
/*                                                                   */
/* Cursor:          ( header ptr ) used to traverse List_ptr.        */
/*                                                                   */
/*********************************************************************/


struct header_entry *list_ptr, *new_config;



    {
      int done = 0;
      struct header_entry * cursor;

      if (EmptyList (list_ptr) == 1)				      /* if the specified configuration list is  */
           list_ptr = new_config;                                           /* empty, tack NEW_CONFIG onto LIST_PTR	  */
      else while (!done)                                                    /* else locate end of the list.            */             
        {

          if (LocateHeader (list_ptr, cursor, new_config -> name) == 0)
             {
                new_config -> last_header = cursor;
                cursor -> next_header = new_config;		      /* if the name of the confiuration doesn't */
                done = 1;					      /* already exist, then append it to end.   */
             }
          else
             {
                PrntError (10, 20, "Configuration name already exists");      /* else rename the configuration.          */
                GetHeaderInfo (new_config);

             }
        }  /*while*/
    }   /*append*/

	 
DeleteConfiguration (list_ptr, name)

/* Procedure DeleteConfiguration:  Deletes a configuration from the  */
/* configuration list.                                               */

/*********************************************************************/
/*                                                                   */
/* Variable Dictionary: DeleteConfiguration                          */
/*                                                                   */
/*                                                                   */
/* Globals: none.                                                    */
/*                                                                   */
/* Parameters: list_ptr, name.                                       */
/*                                                                   */
/* List_ptr:        ( header ptr ) to the beginning of the current   */
/*                  configuration list.  Included so that future     */
/*                  versions may have multiple lists of configs.     */
/*                                                                   */
/* name:            ( char[10] ) string which will be used to locate */
/*                  the position of the target for deletion.         */
/*                                                                   */
/* Locals: cursor.                                                   */
/*                                                                   */
/* Cursor:          ( header ptr ) used to traverse config list.     */
/*                                                                   */
/*********************************************************************/


struct header_entry *list_ptr;
char name [];


    {
      struct header_entry *cursor;
		struct header_entry *dflt;

      if (LocateHeader (list_ptr, cursor, name) == 1)                 /* if target is in list then delete */
        {

         if (((EmptyList (cursor -> next_header)) == 1) &&
            ((EmptyList (cursor -> last_header)) == 1))
           {
            
              list_ptr = (struct option_entry *) NULL;
              dflt = list_ptr;
 
           }
         else if (cursor == list_ptr)
           {
           
             if (!EmptyList (list_ptr -> next_header) == TRUE)	
               ScrollConfiguration (list_ptr, 1);
             else ScrollConfiguration(list_ptr, 0);            
           }
             
         else cursor -> last_header = cursor -> next_header;
         if (cursor = dflt)
           dflt = (struct header_entry *)NULL;
		  }
      else
        PrntError (10, 20, "Specified configuration doesn't exist");  /* else print error message.        */
    }							  


	 
DeleteElement (list_ptr, row, col)

/* Procedure DeleteElement:  Deletes a label entry from the given    */
/*	configuration if it exists.                                */

/*********************************************************************/
/*                                                                   */
/* Variable Dictionary: DeleteElement                                */
/*                                                                   */
/*                                                                   */
/* Globals: none.                                                    */
/*                                                                   */
/* Parameters: List_ptr, row, col.                                   */
/*                                                                   */
/* List_ptr:        ( header ptr ) to the beginning of the target    */
/*                  configuration from which the entry will be       */
/*                  deleted.  This allows for multiple definition    */
/*                  lists.                                           */
/*                                                                   */
/* Row,Col:         ( int ) which are used to locate target element. */
/*                                                                   */
/*	Locals: cursor, trailor, location_code.                    */
/*                                                                   */
/* Cursor,Trailor   ( entry ptrs ) which are used to traverse the    */
/*                  definition list.                                 */
/*                                                                   */
/* Location_code:   ( int ) used to describe if the element is at    */
/*                  beginning of a row, or in the list at all.       */
/*                                                                   */
/*                  location_code = 0  -> search failed              */
/*                     "     "    = 1  -> found (beginning of a row) */
/*                     "     "    = 2  -> found (middle of a row)    */
/*                                                                   */
/*********************************************************************/


struct header_entry *list_ptr;
int row, col;

    {
      struct label_entry *cursor, *trailer;
      int location_code = 0;

      if (LocateListPosition (row, col, list_ptr,	                    /* if the target is in the list */
                              cursor, trailer, location_code) == 1)
        {
          switch (location_code)                                      /* then delete.                 */
             {
                case 1:                                               /* the first element in a row   */
                   trailer -> next_row = cursor;  /**** ???????????? *******/
                   cursor -> next_col -> next_row = cursor -> next_row;
                   break;

                case 2:                                               /* or in the middle, end of row.*/
                   trailer -> next_row = cursor;
                   break;
             }   /*switch*/
        }   /*if*/
      else
        {
          PrntError (10, 20, "Specified entry does not exist.");      /* else print error message     */

        }   /*else*/
    }   /*delete*/



int EmptyEntry (entry_ptr)

/* Function EmptyEntry: This function checks a label_entry pointer to see */
/* if it has a null value.  Returns (1) if TRUE.     		    */
/* Further documentation not necessary.    			    */

struct label_entry *entry_ptr;

    {
      if (entry_ptr = (struct label_entry *) NULL)
        return (1);
      else return (0);
    }

	 
	 

/* Function EmptyList: This function checks a header pointer for nullity */
/* and returns a value of (1) if true.				   */
/* No further documentation necessary.			             */

int EmptyList (header_ptr)
struct header_entry *header_ptr;
{
if (header_ptr = (struct header_entry * ) NULL)
	return (1);
else return (0);
}

	 
	 
FillLabelWindow (list_ptr, label)

/* Procedure FillLabelWindow:  This procedure translates a given label   */
/* configuration and places all information appropriately in the open    */
/* label window.						   */

/*************************************************************************/
/*							   */
/* Variable Dictionary: FillLabelWindow.			   */
/*							   */
/* Parameters: list_ptr, label.				   */
/*							   */
/* list_ptr:  	( header ptr ) to the configuration to be translated.*/
/*							   */
/* label:		( label ) to which the translated entries are to be  */
/*		 displayed. All non-text entries will be in REVERSE  */
/*		 video.					   */
/*					    		   */
/* Locals: count, cursor, trailor, row_ptr.			   */
/*							   */
/* count: 	( int ) loop control index.			   */
/*							   */
/* cursor:	( entry ptr ) scrolled to each entry to be displayed.*/
/* 							   */
/* trailor:	( entry ptr ) used as dummy in this procedure        */
/*							   */
/* row_ptr: 	( entry ptr ) used by LocateListPosition to scroll.  */
/*							   */
/*************************************************************************/


WINDOW label;
struct header_entry *list_ptr;


    {
      int count;
      struct label_entry *cursor, *trailer, *row_ptr;

      ResetScrollList (row_ptr, cursor, list_ptr);
      while ((!EmptyEntry(row_ptr)) && (!EmptyEntry(cursor)))
        {
          DisplayEntry(cursor, label);
          ScrollList (row_ptr, cursor, trailer);
        }
    }

	 
FillNewEntry (row, col, option, attr, width, new_entry)

/* Procedure FillNewEntry: Fills a new label entry with supplied values.  */
/* No further documentation necessary.				    */


struct label_entry *new_entry;
int row, col;
struct option_entry *option;

    {
      new_entry -> row = row;
      new_entry -> col = col;
      new_entry -> code = attr;
      /**** ???????? new_entry -> option_entry = option; ????? ******/
      new_entry -> wdth = width;
      new_entry -> next_row = (struct label_entry *) NULL;
      new_entry -> next_col = (struct label_entry *) NULL;
    }

	 
	 
GetLabelFile (list_ptr, filename, option_list)

/* Procedure GetLabelFile:  Reads existing configuration file from disk */
/* and constructs a corrosponding configuration list.                   */

/************************************************************************/
/*							  */
/* Variable Dictionary:  GetLabelFile.				  */
/*							  */
/* Parameters: list_ptr, filename, option_list.			  */
/*							  */
/* list_ptr:	( header ptr ) to which all information from the    */
/*		 format file will be appended.		  */
/*							  */
/* filename:	( char[] ) string which contains the name of the    */
/* 		 label format file to be opened.		  */
/*							  */
/* option_list:	( option ptr ) to the option list corrosponding the */
/*	  	 input file.  This is supplied so that option ptrs  */
/*		 can immediately be assigned to new entries.        */
/*							  */
/*							  */
/* Locals: new_header, new_entry, cursor, done, dun, infile. 	  */
/*							  */
/* new_header:	( header ptr ) used as allocation pointer for new   */
/*		 headers as they come in from the file.             */
/*							  */
/* new_entry:	( entry ptr ) used as allocation pointer for new    */
/*		 entries as they come in.			  */
/*							  */
/* cursor: 	( option ptr ) which is used to locate corrosponding*/
/*		 option entries and assign option pointers to new   */
/*		 entries.					  */
/*							  */
/* done, dun:	( int ) used as boolean loop control.		  */
/*							  */
/* infile:	( file ptr ) for opening input file.		  */
/*							  */
/************************************************************************/



struct header_entry *list_ptr;
char filename[];
struct option_entry *option_list;

    {
      struct header_entry *new_header;
      struct label_entry *new_entry;
      struct option_entry *cursor;
      int done, dun = 0;
      FILE *infile;

      if ((infile = fopen (filename, "r")) == NULL)
        PrntError (10,10,"File cannot be opened for input");
      else while (!done)
        {
          AllocateHeader (new_header);
          if (fread (new_header, sizeof (struct header_entry),1,infile) == 1)
            done = 1;
          else AppendConfiguration (list_ptr, new_header);
          dun = 0;
          while (!dun)
            {
               AllocateEntry (new_entry);
               if (fread (new_entry, sizeof (struct label_entry),1,infile) == 1)
                  {
                     if (new_entry -> row != -1)
                       {
                       	
                          InsertEntry (new_header, new_entry);
                          ResetOptionScroll (option_list, cursor);
                          LocateOption2 (option_list, cursor, new_entry -> code);
                       } 
                     else dun = 1;
                  }  /*if*/
               else
                  {
                     dun = 1;
                     done = 1;
                  }  /*else*/
            }  /*while*/
        }  /*while*/
    }  /*get label*/

InsertEntry (list_ptr, new_entry)

/* Procedure InsertEntry:  The supplied new label entry is inserted in the */
/* correct list postion in the configuration LIST_PTR.    		     */

/***************************************************************************/
/*							     */
/* Variable Dictionary: InsertEntry.				     */
/*							     */
/*							     */
/* Parameters: list_ptr, new_entry				     */
/*							     */
/* list_ptr:	( header ptr ) to the list in which the new entry is   */
/*		 to be inserted.				     */
/*							     */
/* new_entry:	( entry ptr ) to the entry to be inserted to the list. */
/*							     */
/*							     */
/* Locals: location_code, cursor, trailor.			     */
/*							     */
/* location_code:	( int ) which will specify the position in which the   */
/*		 new entry is to go.			     */
/*                   location_code = 0  -> beginning of list_ptr.          */
/*		   "       "   = 1  -> beginning of a row.	     */
/*		   "       "   = 2  -> middle of a row.		     */
/*							     */
/* cursor:	( entry ptr ) to the element which new_entry will be   */
/*		 placed before.				     */
/*							     */
/* trailor:	( entry ptr ) to the element which new_entry will be   */
/*		 placed after.				     */
/*							     */
/***************************************************************************/


struct label_entry *list_ptr;
struct label_entry *new_entry;


    {
      int location_code = 0;
      struct label_entry *cursor, *trailer;
      if (EmptyEntry (list_ptr -> next_row) == TRUE)
        {
           list_ptr -> next_row = new_entry;
           return;
        }   
       
      if (LocateListPosition (new_entry -> row, new_entry -> col,
                      list_ptr, cursor, trailer, location_code) == 1)
        {
          PrntError (10, 20, "Entry already exists at location .");
          return;
        }
      else
        {
          switch(location_code)
             {
                case 0:
                   trailer -> next_row = new_entry;
                   new_entry -> next_row = cursor;
                   break;

                case 1:
                   trailer -> next_row = new_entry;
                   new_entry -> next_row = cursor -> next_row;
                   new_entry -> next_col = cursor;
                   break;

                case 2:
                   trailer -> next_col = new_entry;
                   new_entry -> next_col = cursor;
                   break;
             }
        }
   }
		  
		  
int LocateHeader (list_ptr, cursor, name)

/* Function LocateHeader:  Attempts to find a header in the header list */
/* pointed to by list_ptr.  The search is conducted by name. If it      */
/* is successful a value of (1) is returned and the header pointer      */
/* cursor is alligned to the located header.         		  */

struct header_entry *list_ptr, *cursor;
char name[];


    {
      int done, inlist = 0;
      ResetScrollConfiguration(list_ptr, cursor);

      if (!EmptyList(cursor) == TRUE)
        while (!done)
          {
             if (strcmp(name, cursor -> name) == TRUE)
                {
                   done = 1;
                   inlist = 1;
                }
             else if (EmptyList(cursor -> next_header) == TRUE)
                {
                   done = 1;
                   inlist = 0;
                }
             else ScrollConfiguration (cursor, 1);
          }  /*while*/
        return(inlist);
    }  /*locate*/

	 
	 
int LocateListPosition (row, col, list_ptr, cursor, trailer, location_code)

/* Procedure LocateListPosition:  This procedure attempts to locate a   */
/* label entry in the configuration pointed to by list_ptr.  The search */
/* is conducted by row and column number.  If successful, a value of (1)*/
/* will be returned and cursor will point to the element located.  If   */
/* the search is unsuccessful, then cursor will point to the element    */
/* after the position specified by (row,col) and trailor the entry after*/

/************************************************************************/
/*							  */
/* Variable Dictionary: LocateListPosition			  */
/*							  */
/*							  */
/* Parameters: row, col, list_ptr, cursor, trailor, location_code.      */
/*							  */
/* row, col:	( int ) which specify the row and column location   */
/*		 of the target element.		 	  */
/*							  */
/* list_ptr: 	( header ptr) to the label configuration list to    */
/*		 search.					  */
/*							  */
/* cursor:	( entry ptr ) alligned to the target element if the */
/*		 search is successful, or to the element after the  */
/*		 position specified by (row,col) if unsuccessful.   */
/*							  */
/* trailor:	( entry ptr ) alligned to the element before the    */
/*		 position specified by (row,col).  Used for pointer */
/*		 reassignment in DeleteElement and InsertEntry.     */
/*							  */
/* location_code	( int ) Specifies the relative position of the      */
/*		 target element.				  */
/*							  */
/*							  */
/* Locals: inlist, dummy, done, thru.				  */
/*							  */
/* inlist:	( int ) return value. (1) if successful.            */
/*							  */
/* dummy: 	( entry ptr ) unimportant.			  */
/*							  */
/* done, thru:	( int ) boolean loop control.			  */
/*							  */
/************************************************************************/


int row, col, location_code;
struct header_entry *list_ptr;
struct label_entry *cursor, *trailer;


    {
      int done, inlist, thru = 0;
      struct label_entry *dummy;

      ResetScrollList (cursor, trailer, list_ptr);
      while (!done)
        {
          if (EmptyEntry(cursor) == TRUE)
             {
                inlist = 0;
                done = 1;
             }
          else if (cursor -> row > row)
             {
                inlist = 0;
                done = 1;
             }
          else if (cursor -> row = row )
             {
                location_code = 1;
                while (!thru)
                   {
                      if (EmptyEntry(cursor -> next_col) == TRUE)
                         {
                            inlist = 0;
                            thru = 1;
                            done = 1;
                         }  /*if*/
                      else if (cursor -> col > col)
                         {
                            inlist = 0;
                            thru = 1;
                            done = 1;
                         }
                      else if (cursor -> col = col)
                         {
                            inlist = 1;
                            thru = 1;
                            done = 1;
                         }
                      else
                         {
                            ScrollList (dummy, cursor, trailer);
                            location_code = 2;
                         }
                   }  /*while*/
              }
          else
             {
                trailer = cursor;
                cursor = cursor -> next_row;
             }  /*else*/
        }  /*while*/

      return(inlist);
    }  /*locate*/
	 
	 
	 


/* Function LocateOption:  This procedure searches a specific option    */
/* list for the option specified by opt_number. If sucessful, a (1) is  */
/* returned and the pointer cursor is positioned on the target.         */
/* No further documentation necessary.			 	  */

int LocateOption (option_list, cursor, opt_number)
struct label_entry *option_list, *cursor;
int opt_number;

  {
   int count;
   ResetOptionScroll (option_list, cursor);
   for (count = 1; count < opt_number + 1; count++)
      ScrollOptionList (cursor);
  	return(1);
  }


LocateOption2 (option_list, cursor, attb)

/* Procedure LocateOption2: Unlike the function LocateOption, this   	  */
/* procedure searches a given optionlist by the character code used to  */
/* identify it on the label window.  No values are returned, but if     */
/* successful, cursor is positioned on the target.		  */
/* No further documentation necessary.				  */

struct option_entry *option_list, *cursor;
char attb;
  {
   int found = 0;
   ResetOptionScroll (option_list, cursor);
   while (!cursor -> code == attb)
      ScrollOptionList (cursor);

  } 


int PrntError (row, col, message)

/* Procedure PrntError:  More a programmers debugging tool, PrntError  */
/* prints an error message in a windo at row, col, and waits for any   */
/* key to be pressed to continue.				 */
/* No further documentation required.				 */

int row, col;
char message[];

    {
      WINDOW err;

      defs_wn(&err,row,col,row+4,80-col, BDR_LNP);
      sw_popup(YES,&err);
      set_wn(&err);
      mv_cs(0,0,&err);
      v_st((char *) message,&err);
      mv_cs(1,0,&err);
      v_st((char *)"Press any key to continue", &err);
      ki();
      unset_wn(&err);
    }

	 
ResetOptionScroll (option_list, cursor)

/* Procedure ResetScrollOption:  This procedure resets the option ptr  */
/* cursor to the beginning of the supplied option list.                */
/* No further documentation necessary.				 */

struct option_entry *option_list, *cursor;

    {
      cursor = option_list;
    }

	 
	 
ResetScrollConfiguration (list_ptr, cursor)

/* Procedure ResetScrollConfiguration:  This procedure scrolls the header */
/* list pointer back to the beginning of the configuration list.	    */
/* No further documantation necessary.			   	    */

struct header_entry *list_ptr, *cursor;


    {
      cursor = list_ptr;
      if (!EmptyList(cursor) == TRUE)
        while (!EmptyList(cursor -> last_header) == TRUE)
           ScrollConfiguration(cursor, 0);
    }



ResetScrollList (row_ptr, col_ptr, list_ptr)

/* Procedure ResetScrollList:  This procedure resets to label entry ptrs */
/* to the beginning or a supplied configuration header.		   */

struct label_entry *row_ptr, *col_ptr;
struct header_entry *list_ptr;


    {
      if (!EmptyEntry(list_ptr) == TRUE)
        {
        /*** ????????  row_ptr = list_ptr -> next_row; ????? ******/
          col_ptr = row_ptr;
        }
      else
          PrntError (10, 20, "Specified label configuration is undefined");
    }

	 
	 
SaveLabelFile (list_ptr, filename, changed)

/* Procedure SaveLabelFile:  This procedure saves all information contained */
/* in the configuration list pointed to by list_ptr to a formated file. Upon*/
/* completion of the save, the variable changed to true.                    */


struct header_entry *list_ptr;
char filename[];
int changed;

    {
      struct header_entry *hcur;
      struct label_entry *lcur, *ltrai, *lrow, *filler;
      int done, dun = 0;
      FILE *outfile;

      AllocateEntry (filler);
      FillNewEntry ("", -1, (struct option_entry *)NULL, 0, "", filler);
      outfile = fopen (filename, "w");
      ResetScrollConfiguration (list_ptr, hcur);
      ResetScrollList (lrow, lcur, list_ptr);
      while (!done)
        {
          if (!EmptyList(hcur) == 1)
            fwrite (hcur, sizeof(struct header_entry), 1, outfile);
          dun = 0;
          while (!dun)
            {
               if ((!EmptyEntry(lcur) == 1) || (!EmptyEntry(lrow) == 1))
                  {
                   fwrite (lcur, sizeof(struct label_entry), 1, outfile);
                     ScrollList (lrow, lcur, ltrai);
                  }
               else dun = 1;
            }
          if ((EmptyList (hcur -> next_header) == 1))
            done = 1;
          else
            {
               ScrollConfiguration (hcur, 1);
               ResetScrollList (lrow, lcur, hcur);	 /***** ????????? *****/
               fwrite(filler, sizeof (struct label_entry),1,outfile);
            }
        }
		fclose(outfile);
     changed = 0;
    }

	 
	 
ScrollConfiguration (cursor, direction)

struct header_entry *cursor;


    {
      if (!EmptyList(cursor) == TRUE)
        switch (direction)
          {
             case 1:
                if (!EmptyList(cursor -> next_header) ==TRUE)
                   cursor = cursor -> next_header;
                break;

             case 2:
                if (!EmptyList(cursor -> last_header) == TRUE)
                   cursor = cursor -> last_header;
                break;
          }               /*switch*/
    }   /*scroll*/



ScrollList (row_ptr, col_ptr, trailer)

struct label_entry *row_ptr, *col_ptr, *trailer;


    {

	 struct label_entry *cursor;

      if (EmptyEntry(col_ptr) == TRUE)
        {
          if (EmptyEntry(row_ptr) == TRUE)
             return;
          else
             {
                trailer = row_ptr;
                row_ptr = row_ptr -> next_row;
                cursor = row_ptr;
             }
        }
      else
        {
          trailer = cursor;
          cursor = cursor -> next_col;
        }
    }

	 
	 
ScrollOptionList (cursor, direction)
struct option_entry *cursor;
int direction;

    {
      if (cursor != (struct option_entry * ) NULL)
        switch(direction)
          {
             case 1:
                if (cursor -> next_option != (struct option_entry * ) NULL)
                   cursor = cursor -> next_option;
                break;

             case 2:
                if (cursor -> last_option != (struct option_enry * ) NULL)
                   cursor = cursor -> last_option;
                break;
          }

  }






AddOptionMenu (option_menu, item)
MFILEPTR option_menu;
char * item;
{
mf_rwrpl(item, LAST_ROW, option_menu);
}


CloseLabelWindow(label, header, changed)

WINDOW label;
struct header_entry *header;
int changed;

  {
   struct label_entry *temp_entry;
   int i, j, k;
   int y, x;
   int c, atb;
   char strng[50];
     
   strcpy(strng, " ");
   k = 1;
   if (changed)

   for (i = 1; i < header -> cols+1; i++)
     for (j = 1; j < header -> rows+1; j++)
        {

         mv_csr (j, i, &label);
         atb = vo_att(&label);
         c = vo_ch (&label);
         if ((atb == REVERSE)||((char) c == ' ')||(j == header -> cols))
           {
       
    	    if (strng[1] != ' ')
	      {
             
                   AllocateEntry (temp_entry);
                   if (!atb == REVERSE)
                     strng[k] = (char) c;
                                    
                   FillNewEntry (x, y, (struct option_entry *) NULL, 't', strng, 0, temp_entry);
                   strcpy(strng, " ");  
                   k = 0;

                }
           }
         else
           {
  
            strng[k] = (char) c;
            k++;

           }   
        }
   changed = 0;
  }




CommandProcessor(x, y, changed, label, cmenu_window, omenu_window,
                 cmem_file, omem_file, option_list, header)

int x, y, changed;
WINDOW label, cmenu_window, omenu_window;
MFILEPTR cmem_file, omem_file;
struct header_entry *header;
struct option_entry *option_list; 

   {
    struct label_entry  *new_entry;
    struct option_entry *cursor;
    struct header_entry *new_label;
	 struct header_entry *dflt;
    int oselect, cselect;
    char answer = 'n';
	 int num_opts = 7;

    if (isset_wn (&cmenu_window))
      unset_wn (&cmenu_window);

    cselect = menu2 (&cmenu_window, 8, 28, 1, 1);
    SetCommandWindow (&omenu_window, omem_file);
      switch (cselect)
        {
          case 1:
            if (isset_wn (&omenu_window))
              unset_wn (&omenu_window);
 																 /*** ????? *****/
            if (oselect = menu2 (&omenu_window, num_opts, 28, 1, 1) != 0)
              if (LocateOption (option_list, cursor, oselect) == 1)
                {

                   AllocateEntry (new_entry);
                   FillNewEntry (x ,y , cursor -> option, cursor -> code,
                                 cursor -> width, new_entry);
                   InsertEntry (header, new_entry);
                   DisplayEntry (new_entry, label);
                   changed = 1;

                }
              else PrntError (10, 20, "WARNING: A SYSTEM ERROR HAS OCCURRED");
            else
              bell();
            SetOptionWindow (omenu_window, omem_file);

            break;
          
          case 2:
            
            DeleteElement (x, y, header);
            CloseLabelWindow (label, header, changed);
            cl_wn (&label);
            FillLabelWindow (header, label);
            mv_cs (0, 0, &label);
            pl_csr (&label);
            break;

          case 3: 

            CloseLabelWindow (label, header, changed);
            unset_wn (&label);            
            AllocateHeader (new_label);
            GetHeaderInfo (new_label);
            AppendConfiguration (header, new_label);
            header = new_label;
            DefineLabelWindow (label, header);
            set_wn (&label);
            x = 0;
            y = 0;
            mv_cs (0, 0, &label);
            pl_csr(&label);
            changed = 1;
            break;

         case 4:

            if (Quser ("Are you sure"))
              {
     
                DeleteConfiguration (header);
                UpdateConfigWindow (dflt);
                changed = 1;

              }
            break;


         case 5:

            vs_mf (K_ESC, &omenu_window);
            break;


         case 6:
            
            CloseLabelWindow (label, &header, changed);
          /***** ???????  SaveLabelFile (header, fname); ?????? ******/
            break;

         case 7:
            
            dflt = header;
            break;

         case 8:

            GetHeaderInfo (header);
            CloseLabelWindow (label, header, changed);
            unset_wn (&label);
            DefineLabelWindow (label, header);
            set_wn (&label);
            FillLabelWindow (header, &label);
            x = 0;
            y = 0;
            mv_cs (0, 0, &label);
            pl_csr (&label);
            break;

       }
  }

                 

int ConstructString(option, strng)
struct option_entry *option;
char *strng;
{
char tmp[3];
sprintf(tmp, "%c", option->code);
strcat (strng, option -> optstr);
strcat (strng, tmp);
return(1);
}




int CreateCommandMenu(com_menu)
MFILEPTR com_menu;
{
char *command[8];
int  count;
int  retval = 0;

   command[1] = "Define new label entry";
   command[2] = "Remove existing label";
   command[3] = "Define new label";
   command[4] = "Remove existing label";
   command[5] = "Scroll options list";
   command[6] = "Save changes to disk";
   command[7] = "Select new default label";
   command[8] = "Update header information";

   for (count = 1;count < 9;count++)
      {
		if ((retval = (mf_rwrpl (command[count], LAST_ROW, com_menu))) <= 0)
	      break;
		}
return(0);
}
 


int DefineHeaderForm(label_header)
struct header_entry *label_header;
{
DFORMPTR header_form;

header_form = fm_def(0, 51, 9, 30, LNORMAL, BDR_LNP);

fld_def(1, 1, "Label Name: ", FADJACENT, "XXXXXXXXXX", F_STRING, (char *) label_header -> name, header_form);
ftxt_def(1, 0, (char *) "------Label Dimensions------", LHIGHLITE, header_form);
fld_def(2, 1, "Length (rows): ",FADJACENT, "99", F_INT, (char *) &(label_header -> rows), header_form);
fld_def(3, 1, "Width (columns): ",FADJACENT, "99", F_INT, (char *) &(label_header -> cols), header_form);
ftxt_def(5, 0, (char *) "------Printer Location------", LHIGHLITE, header_form);
fld_def(6, 1, "Line printer row: ", FADJACENT, "99", F_INT, (char *) &(label_header -> p_rows), header_form);
fld_def(7, 1, "Line printer col: ", FADJACENT, "99", F_INT, (char *) &(label_header -> p_cols), header_form);
fld_def(9, 0, "---------Application--------", FBELOW, def_pic('X',28), F_STRING, (label_header -> use), header_form);
return(0);
}


int DefineLabelWindow(label, header)
WINDOW label;
struct header_entry * header;
  {
   defs_wn (&label, 11 - (int)((header -> rows) / 2), 23 - (int)(header -> cols / 2),
             header -> rows, header -> cols, BDR_LNP);

	sw_name ("Label Configuration", &label);
   sw_namelocation (TOPCENTER, &label);
   sw_popup (ON, &label);
   sw_plcsr (ON, &label);
   color_wn (BLACK, WHITE, &label);
	return(0);
  }



int DisplayEntry(entry, label)
struct label_entry *entry;
WINDOW label;
{
int atb, count;

mv_cs (entry -> row, entry -> col, &label);
atb=vo_att(&label);
if ((entry->code == 't') && (atb != REVERSE))
	{
	v_st((char *) entry->txt, &label);
	}
else 
	{
	for (count = 0; count < entry -> wdth; count++)
		{
		mv_cs (entry -> row, entry -> col + count, &label);
		atb=vo_att(&label);
		if (atb != REVERSE)
			{
			v_att(REVERSE, &label);
			v_ch (entry -> code, &label);
			}
		else count = 50;
		}
	}
return(0);
}

  
       
int GetHeaderInfo(header)
struct header_entry * header;
  {
DFORMPTR header_form;
int done = 0;
   DefineHeaderForm(header_form);
   while (!done)
     {
      
        fm_proc(0, header_form);
        if (header -> rows > 23)
          PrntError(10, 20, "Label length exceeds maximum 23 rows");
        else if (header -> cols > 48)
          PrntError(10, 20, "Label width exceeds maximum 48 columns");
        else if (header -> rows < 1) 
          PrntError(10, 20, "Improper Label Length");
        else if (header -> cols < 20) 
          PrntError(10, 20, "Improper Label Length");  
        else done = 1;

     }
	return(0);
  }

      


int DefineCommandWindow (command_window)
WINDOW command_window;
  {    
   defs_wn (&command_window, 0, 51, 9, 30, BDR_LNP); 
   sw_name ("Commands", &command_window);
   sw_namelocation (TOPCENTER, &command_window);
   sw_popup (ON, &command_window);
   color_wn (RED, BLUE, &command_window);
	return(0);
  }

int DefineOptionWindow (option_window)  
WINDOW option_window;
  {
   defs_wn (&option_window, 10, 51, 10, 30, BDR_LNP);
   sw_name ("Options", &option_window);
   sw_namelocation (TOPCENTER, &option_window);
   sw_popup (ON, &option_window);
   color_wn (WHITE, BLUE, &option_window);
	return(0);
  }


  
int DefineConfigWindow (config_window)
WINDOW config_window;
  {
   defs_wn (&config_window, 21, 51, 5, 30, BDR_LNP);
   sw_name ("Labels", &config_window);
   sw_namelocation (TOPCENTER, &config_window);
   sw_popup (ON, &config_window);
   color_wn (BLACK, BLUE, &config_window);
	return(0);
  }



int LabelProcess(label, header, changed)
WINDOW label;
struct header_entry * header;
int changed;

  {
   int c, atb;
   int done=0;
     
   while (!done)
     {
      switch (c = ki())
        {

         case K_ENTER:
 
           CommandProcessor (label.r, label.c, changed);
           break;
                       
         case K_ESC:

           goto END;
           break;
         case K_BACK:
           atb=vo_att(&label);
           if (label.c > 0)
             {     
             
               if (!atb == REVERSE)
                 {
    
                   v_ch (' ', &label);
                   changed = 1;

                 }
              label.c--;
              pl_csr (&label);
   
             }
           else 
             bell();
           break;

         case -K_LEFT:

           if (label.c >0)
             {
 
              label.c--;
              pl_csr (&label);

             }

           break;
						 

         case -K_RIGHT:

           if (label.c < header -> cols)
             {

                label.c++;
                pl_csr (&label);

             }

           break;

         case -K_UP:

           if (label.r >0)
             {

              label.r--;
              pl_csr (&label);

             }

          break;

        case -K_DN:

           if (label.r < header -> rows)
             {

              label.r++;
              pl_csr (&label);

             }

           break;

         case -K_PGUP:	
         
           CloseLabelWindow (label, header, changed);
           ScrollConfiguration (header, 0);
           UpdateConfigWindow (header);
           DefineLabelWindow (label, header);
           set_wn (&label);
           FillLabelWindow (label, header);

           break;

         case -K_PGDN:

           CloseLabelWindow (label, header, changed);
           ScrollConfiguration (header, 1);
           UpdateConfigWindow (header);
           DefineLabelWindow (label, header);
           set_wn (&label);
           FillLabelWindow (label, header);

           break;

         default:
           atb=vo_att(&label);
           if ((c < 0) && (atb != REVERSE))
             {

                v_ch (c, &label);
                if (label.c == header -> cols)
                  {

                     if (label.r != header -> rows)
                       label.r++;
                     else label.r = 0;

                 label.c = 0;

                }
                
              else label.c++;
              pl_csr (&label);
           }
           else 
             bell();
   
           break;

        }
   }
END:
return(0);
} 




int Quser (question)
char *question;
   {
    DFORMPTR Q;
    DFIELDPTR Qf;
    char answer = 'N';
    Q = fm_def (10, 1, 10, 70, LNORMAL, BDR_0P);
    Qf = fld_def (0, 1, question, FADJACENT, "!", F_CHAR, (char *) &answer, Q);
    fm_proc (0, Q);
	 if (answer == 'Y') return(1);
    else return (0);
   }

     
int SetCommandWindow (command_window, memory_file) 
WINDOW command_window;
MFILEPTR memory_file;
  {
   sw_mf (memory_file, &command_window);
   pl_mfwn (0, 1, &command_window);
   v_mf (&command_window);
	return(0);
  }


int SetOptionWindow (option_window, memory_file)
WINDOW option_window;
MFILEPTR memory_file;
  {
   sw_mf (memory_file, &option_window);
   pl_mfwn (0, 1, &option_window);
   v_mf (&option_window);
	return(0);
  }
    


  
int UpdateConfigWindow(header, cwindow)
struct header_entry *header;
WINDOW cwindow;
  {
  struct header_entry *dflt;
  cl_wn (&cwindow);
   if (emptylist(header -> last_header) != 1)
     {

        mv_cs (0, 5, &cwindow);
        v_st ((char *) header -> last_header -> name, &cwindow);
        if (header -> last_header == dflt)
          {
            mv_cs (0, 18, &cwindow);
            v_st ((char *) "(def)", &cwindow);
          }
     }

   mv_cs (1, 1, &cwindow);
   v_st ((char *) "===>", &cwindow);
   if (emptyList(header)!=1)
     {

        mv_cs (1, 5, &cwindow); 
        v_st ((char *) header -> name, &cwindow);
        if (header == dflt)
          {
           mv_cs (1, 18, &cwindow);
           v_st ((char *) "(def)", &cwindow);
          }
     }

   if (emptyList(header -> next_header) != 1)
     {
 
        mv_cs (2, 5, &cwindow);
        v_st ((char *) header -> next_header -> name, &cwindow);
        if (header -> next_header = dflt)
          {
           mv_cs (2, 18, &cwindow);
           v_st ((char *) "(def)", &cwindow);
          }
     }
return(0);
}




           








