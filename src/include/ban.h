#ifndef _BAN_H
#define _BAN_H

#define BANNED_NAME_FILE  "adm/banned"

#ifndef __BAN_C__
/* extern char                             banned_names[MAX_STRING_LENGTH]; */	/* dumbfuck asshole and friends */
#endif

int                                     banned_name(char *name);
void					load_bans(void);
void					unload_bans(void);
void                                    bans_to_sql(void);
void                                    do_ban(struct char_data *ch, char *argument, int cmd);
void                                    do_unban(struct char_data *ch, char *argument, int cmd);

#endif
