/*
 * file: actwiz.c , Implementation of commands.           Part of DIKUMUD
 * Usage : Wizard Commands.
 * Copyright (C) 1990, 1991 - see 'license.doc' for complete information.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <ctype.h>
#include <time.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"
#include "include/comm.h"
#include "include/interpreter.h"
#include "include/handler.h"
#include "include/db.h"
#include "include/spells.h"
#include "include/limits.h"
#include "include/constants.h"
#include "include/spell_parser.h"
#include "include/board.h"
#include "include/whod.h"
#include "include/reception.h"
#include "include/spec_procs.h"
#include "include/multiclass.h"
#include "include/act_skills.h"
#include "include/act_info.h"
#include "include/fight.h"
#include "include/hash.h"
#include "include/weather.h"
#include "include/modify.h"
#define _ACT_WIZ_C
#include "include/act_wiz.h"

void do_polymorph(struct char_data *ch, char *argument, int cmdnum)
{

}

void do_highfive(struct char_data *ch, char *argument, int cmd)
{
  char buf[80];
  struct char_data *tch;

  if (argument) {
    only_argument(argument, buf);
    if ((tch = get_char_vis(ch, buf)) != 0) {
      if ((GetMaxLevel(tch) >= DEMIGOD) && (!IS_NPC(tch))) {
	aprintf("Time stops for a moment as %s and %s high five.\n\r",
		ch->player.name, tch->player.name);
      } else {
	act("$n gives you a high five", TRUE, ch, 0, tch, TO_VICT);
	act("You give a hearty high five to $N", TRUE, ch, 0, tch, TO_CHAR);
	act("$n and $N do a high five.", TRUE, ch, 0, tch, TO_NOTVICT);
      }
    } else {
      cprintf(ch, "I don't see anyone here like that.\n\r");
    }
  }
}

void do_rentmode(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_STRING_LENGTH];
  double it;
  FILE *pfd;

  if(IS_NPC(ch)) {
    cprintf(ch, "You cannot toggle rent costs.\n\r");
    return;
  }
  if(argument && *argument) {
    only_argument(argument, buf);
    if(sscanf(buf, " %lf ", &it) == 1)
      RENT_RATE= it;
    sprintf(buf, "Rent now costs %f normal.", RENT_RATE);
    cprintf(ch, "%s\n\r", buf);
    log(buf);
  } else {
    if (RENT_RATE != 0.0) {
      cprintf(ch, "Rent is now free!\n\r");
      log("Rent cost is now ZERO.");
      RENT_RATE = 0.0;
    } else {
      cprintf(ch, "Rent is now normal.\n\r");
      log("Rent cost is now normal.");
      RENT_RATE = 1.0;
    }
  }
  if(!(pfd= fopen(RENTCOST_FILE, "w"))) {
    log("Cannot save rent cost!");
  } else {
    fprintf(pfd, "%f\n", RENT_RATE);
    fclose(pfd);
  }
}

void do_wizlock(struct char_data *ch, char *argument, int cmd)
{

  if (IS_NPC(ch)) {
    cprintf(ch, "You cannot WizLock.\n\r");
    return;
  }
  if (WizLock) {
    cprintf(ch, "WizLock is now off\n\r");
    log("Wizlock is now off.");
    WizLock = FALSE;
  } else {
    cprintf(ch, "WizLock is now on\n\r");
    log("WizLock is now on.");
    WizLock = TRUE;
  }
}

void do_emote(struct char_data *ch, char *argument, int cmd)
{
  int i;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch) && (cmd != 0) && (cmd != 214))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    cprintf(ch, "Yes.. But what?\n\r");
  else {
    sprintf(buf, "$n %s", argument + i);
    act(buf, FALSE, ch, 0, 0, TO_ROOM);
    if(IS_NPC(ch) || (IS_SET(ch->specials.act, PLR_ECHO))) {
      cprintf(ch, "You emote: '%s'\n\r", argument + i);
    }
  }
}

void do_echo(struct char_data *ch, char *argument, int cmd)
{
  int i;

  if (IS_NPC(ch))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i)) {
    if (IS_SET(ch->specials.act, PLR_ECHO)) {
      cprintf(ch, "echo off\n\r");
      REMOVE_BIT(ch->specials.act, PLR_ECHO);
    } else {
      SET_BIT(ch->specials.act, PLR_ECHO);
      cprintf(ch, "echo on\n\r");
    }
  } else {
    if (IS_IMMORTAL(ch)) {
      reprintf(ch->in_room, ch, "%s\n\r", argument + i);
      cprintf(ch, "Ok.\n\r");
    }
  }
}

void do_system(struct char_data *ch, char *argument, int cmd)
{
  int i;

  if (IS_NPC(ch))
    return;

  for (i = 0; *(argument + i) == ' '; i++);

  if (!*(argument + i))
    cprintf(ch, "That must be a mistake...\n\r");
  else {
    aprintf("\n\r%s\n\r", argument + i);
  }
}

void do_trans(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *victim;
  char buf[100];
  SHORT target;

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf)
    cprintf(ch, "Who do you wich to transfer?\n\r");
  else if (str_cmp("all", buf)) {
    if (!(victim = get_char_vis_world(ch, buf, NULL)))
      cprintf(ch, "No-one by that name around.\n\r");
    else if(GetMaxLevel(victim) > GetMaxLevel(ch)) {
      cprintf(ch, "You are not strong enough to force %s to appear.\n\r",
              NAME(victim));
      cprintf(victim, "%s would like to transfer you.\n\r", NAME(ch));
    } else {
      act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
      target = ch->in_room;
      if (MOUNTED(victim)) {
	char_from_room(victim);
	char_from_room(MOUNTED(victim));
	char_to_room(victim, target);
	char_to_room(MOUNTED(victim), target);
      } else {
	char_from_room(victim);
	char_to_room(victim, target);
      }
      act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
      act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
      do_look(victim, "", 15);
      cprintf(ch, "Ok.\n\r");
    }
  } else {			       /* Trans All */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected) {
	victim = i->character;
	if (MOUNTED(victim))
	  Dismount(victim, MOUNTED(victim), POSITION_STANDING);
	target = ch->in_room;
	char_from_room(victim);
	char_to_room(victim, target);
	act("$n disappears in a mushroom cloud.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n arrives from a puff of smoke.", FALSE, victim, 0, 0, TO_ROOM);
	act("$n has transferred you!", FALSE, ch, 0, victim, TO_VICT);
	do_look(victim, "", 15);
      }
    cprintf(ch, "Ok.\n\r");
  }
}

void do_at(struct char_data *ch, char *argument, int cmd)
{
  char command[MAX_INPUT_LENGTH], loc_str[MAX_INPUT_LENGTH];
  int location, original_loc;
  struct char_data *target_mob;
  struct obj_data *target_obj;

  if (IS_NPC(ch))
    return;

  half_chop(argument, loc_str, command);
  if (!*loc_str) {
    cprintf(ch, "You must supply a room number or a name.\n\r");
    return;
  }
  if (!(target_mob= get_char_room_vis(ch, loc_str))) {
    if (!(target_mob= get_char(loc_str))) {
      if (!(target_obj = get_obj_vis_world(ch, loc_str, NULL))) {
        if (!(location= atoi(loc_str))) {
          cprintf(ch, "I have no idea where \"%s\" is...\n\r", loc_str);
          return;
        } else if (!(real_roomp(location))) {
          cprintf(ch, "That room exists only in your imagination.\n\r");
          return;
        }
      } else {
        if ((location= target_obj->in_room) == NOWHERE) {
          cprintf(ch, "The object is not available.\n\r");
          return;
        }
      }
    } else {
      if((location = target_mob->in_room) == NOWHERE) {
        cprintf(ch, "The target mobile is not available.\n\r");
        return;
      }
    }
  } else {
    if((location = target_mob->in_room) == NOWHERE) {
      cprintf(ch, "The target mobile is not available.\n\r");
      return;
    }
  }
  /* a location has been found. */

  original_loc = ch->in_room;
  char_from_room(ch);
  char_to_room(ch, location);
  command_interpreter(ch, command);

  /* check if the guy's still there */
  for (target_mob = real_roomp(location)->people; target_mob; target_mob =
       target_mob->next_in_room)
    if (ch == target_mob) {
      char_from_room(ch);
      char_to_room(ch, original_loc);
    }
}

void do_form(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int loc_nr;
  struct room_data *rp;
  int zone;

  if (IS_NPC(ch))
    return;
  only_argument(argument, buf);
  if (!*buf) {
    cprintf(ch, "Usage: FORM virtual_number.\n\r");
    return;
  }
  if (!(isdigit(*buf))) {
    cprintf(ch, "Usage: FORM virtual_number.\n\r");
    return;
  }
  loc_nr = atoi(buf);
  if (real_roomp(loc_nr)) {
    cprintf(ch, "A room exists with that Vnum!\n\r");
    return;
  } else if (loc_nr < 0) {
    cprintf(ch, "You must use a positive Vnum!\n\r");
    return;
  }
  cprintf(ch, "You have formed a room.\n\r");
  allocate_room(loc_nr);
  rp = real_roomp(loc_nr);
  bzero(rp, sizeof(*rp));
  rp->number = loc_nr;
  if (top_of_zone_table >= 0) {
    for (zone = 0; rp->number > zone_table[zone].top && zone <= top_of_zone_table; zone++);
    if (zone > top_of_zone_table) {
      fprintf(stderr, "Room %d is outside of any zone.\n", rp->number);
      zone--;
    }
    rp->zone = zone;
  }
  sprintf(buf, "%d", loc_nr);
  rp->name = (char *)strdup(buf);
  rp->description = (char *)strdup("New Room\n");
}

void do_goto(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int loc_nr, location, i;
  struct char_data *target_mob, *pers, *v;
  struct obj_data *target_obj;

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf) {
    cprintf(ch, "You must supply a room number or a name.\n\r");
    return;
  }
  if (isdigit(*buf) && NULL == index(buf, '.')) {
    loc_nr = atoi(buf);
    if (NULL == real_roomp(loc_nr)) {
      cprintf(ch, "No room exists with that number.\n\r");
      return;
    }
    location = loc_nr;
  } else if ((target_mob = get_char_vis_world(ch, buf, NULL)))
    location = target_mob->in_room;
  else if ((target_obj = get_obj_vis_world(ch, buf, NULL)))
    if (target_obj->in_room != NOWHERE)
      location = target_obj->in_room;
    else {
      cprintf(ch, "The object is not available.\n\r");
      cprintf(ch, "Try where #.object to nail its room number.\n\r");
      return;
  } else {
    cprintf(ch, "No such creature or object around.\n\r");
    return;
  }

  /* a location has been found. */

  if (!real_roomp(location)) {
    log("Massive error in do_goto. Everyone Off NOW.");
    return;
  }
  if (IS_SET(real_roomp(location)->room_flags, PRIVATE)) {
    for (i = 0, pers = real_roomp(location)->people; pers; pers =
	 pers->next_in_room, i++);
    if (i > 1 && IS_MORTAL(ch)) {
      cprintf(ch, "There's a private conversation going on in that room.\n\r");
      return;
    }
  }
  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n disappears into a cloud of mist.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n disappears into a cloud of mist.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (ch->specials.fighting)
    stop_fighting(ch);
  if (MOUNTED(ch)) {
    char_from_room(ch);
    char_to_room(ch, location);
    char_from_room(MOUNTED(ch));
    char_to_room(MOUNTED(ch), location);
  } else {
    char_from_room(ch);
    char_to_room(ch, location);
  }

  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n appears from a cloud of mist.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n appears from a cloud of mist.", FALSE, ch, 0, 0, TO_ROOM);
  }
  do_look(ch, "", 15);
}

void do_home(struct char_data *ch, char *argument, int cmd)
{
  char buf[MAX_INPUT_LENGTH];
  int location;
  struct char_data *v;

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);
  if (!*buf) {
    location = GET_HOME(ch);
  } else {
    cprintf(ch, "You can't just barge into someone else's home (yet)!\n\r");
    location = GET_HOME(ch);
  }
  if (!real_roomp(location)) {
    cprintf(ch, "Hmmmm... homeless, peniless, but surely not alone.\n\r");
    return;
  }
  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n stretches sensually and elongates into a wisp of vapour.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n stretches sensually and elongates into a wisp of vapour.", FALSE, ch, 0, 0, TO_ROOM);
  }

  if (ch->specials.fighting)
    stop_fighting(ch);
  if (MOUNTED(ch)) {
    char_from_room(ch);
    char_to_room(ch, location);
    char_from_room(MOUNTED(ch));
    char_to_room(MOUNTED(ch), location);
  } else {
    char_from_room(ch);
    char_to_room(ch, location);
  }

  if (IS_SET(ch->specials.act, PLR_STEALTH)) {
    for (v = real_roomp(ch->in_room)->people; v; v = v->next_in_room) {
      if ((ch != v) && (GetMaxLevel(v) >= GetMaxLevel(ch))) {
	act("$n arrives and immediately curls up in $s spot.", FALSE, ch, 0, v, TO_VICT);
      }
    }
  } else {
    act("$n arrives and immediately curls up in $s spot.", FALSE, ch, 0, 0, TO_ROOM);
  }
  do_look(ch, "", 15);
}

void do_apraise(struct char_data *ch, char *argument, int cmd)
{
  char arg1[MAX_STRING_LENGTH];
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct obj_data *j = 0;
  int i;
  int found_one;
  int chance;

  /* for objects */

  /* for rooms */

  /* for chars */

  if (IS_NPC(ch))
    return;

  found_one = 0;

  only_argument(argument, arg1);

  /* no argument */

  if (GET_MANA(ch) < 3) {
    cprintf(ch, "You can't seem to concentrate enough at the moment.\n\r");
    return;
  }
  chance = number(1, 101);

  if (chance > ch->skills[SKILL_APRAISE].learned) {
    cprintf(ch, "You are unable to apraise this item.\n\r");
    GET_MANA(ch) -= 1;
    return;
  }
  GET_MANA(ch) -= 3;

  if (!*arg1) {
    cprintf(ch, "apraise what?\n\r");
    return;
  } else {
    if (ch->skills[SKILL_APRAISE].learned < 50)
      ch->skills[SKILL_APRAISE].learned = MIN(50, ch->skills[SKILL_APRAISE].learned++);

    /* apraise on object */
    if ((j = (struct obj_data *)get_obj_in_list_vis(ch, arg1, ch->carrying))) {
      sprintf(buf, "Object name: [%s]\n\rItem type: ", j->name);
      sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
      strcat(buf, buf2);
      strcat(buf, "\n\r");
      cprintf(ch, buf);

      cprintf(ch, "Can be worn on :");
      sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
      strcat(buf, "\n\r");
      cprintf(ch, buf);

      cprintf(ch, "Weight: %d, Value: %d, Cost/day: %d\n\r",
	      j->obj_flags.weight, j->obj_flags.cost,
	      j->obj_flags.cost_per_day);

      switch (j->obj_flags.type_flag) {
      case ITEM_LIGHT:
	cprintf(ch, "Light hours of Use : [%d]", j->obj_flags.value[2]);
	break;
      case ITEM_WEAPON:
	sprintf(buf, "Weapon Class:");
	switch (j->obj_flags.value[3]) {
	case 0:
	  strcat(buf, "Smiting Class.\n\r");
	  break;
	case 1:
	  strcat(buf, "Stabbing Class.\n\r");
	  break;
	case 2:
	  strcat(buf, "Whipping Class.\n\r");
	  break;
	case 3:
	  strcat(buf, "Slashing Class.\n\r");
	  break;
	case 4:
	  strcat(buf, "Smashing Class.\n\r");
	  break;
	case 5:
	  strcat(buf, "Cleaving Class.\n\r");
	  break;
	case 6:
	  strcat(buf, "Crushing Class.\n\r");
	  break;
	case 7:
	  strcat(buf, "Bludgeoning Class.\n\r");
	  break;
	case 11:
	  strcat(buf, "Piercing Class.\n\r");
	  break;
	default:
	  strcat(buf, "Foreign Class to you....\n\r");
	  break;
	}

	found_one = 0;

	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	  if (j->affected[i].location == APPLY_HITROLL || j->affected[i].location == APPLY_HITNDAM) {
	    found_one = 1;
	    switch (j->affected[i].modifier) {
	    case 1:
	      strcat(buf, "It is well balanced.\n\r");
	      break;
	    case 2:
	      strcat(buf, "It is very well balanced.\n\r");
	      break;
	    case 3:
	      strcat(buf, "It is a superb weapon.\n\r");
	      break;
	    case 4:
	      strcat(buf, "It was forged by the gods.\n\r");
	      break;
	    case 5:
	      strcat(buf, "It should not be in your hands.\n\r");
	      break;
	    default:
	      strcat(buf, "It will crack with the next blow.\n\r");
	      break;
	    }
	  }
	}

	if (!found_one)
	  strcat(buf, "It is common in accuracy.\n\r");

	found_one = 0;
	for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	  if (j->affected[i].location == APPLY_DAMROLL || j->affected[i].location == APPLY_HITNDAM) {
	    found_one = 1;
	    switch (j->affected[i].modifier) {
	    case 0:
	      strcat(buf, "It will surely damage its target.\n\r");
	      break;
	    case 1:
	      strcat(buf, "It looks to be made from a strong metal.\n\r");
	      break;
	    case 2:
	      strcat(buf, "This was forged from a mystical flame.\n\r");
	      break;
	    case 3:
	      strcat(buf, "It has definite magical charms.\n\r");
	      break;
	    case 4:
	      strcat(buf, "This is definitately blessed by the gods.\n\r");
	      break;
	    case 5:
	      strcat(buf, "It is ready to lose its hilt.\n\r");
	      break;
	    default:
	      strcat(buf, "It is checked badly and most likely will break.\n\r");
	      break;
	    }
	  }
	}
	if (!found_one)
	  strcat(buf, "It has a common strength to its making.\n\r");
	cprintf(ch, buf);

	break;
      case ITEM_ARMOR:
	sprintf(buf, "Effective AC points: [%d]\n\rWhen Repaired: [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	if (j->obj_flags.value[0] != 0 && j->obj_flags.value[1] == 0) {
	  strcat(buf, "\n\rYou should take it to be updated at the Blacksmith\n\r");
	}
	cprintf(ch, buf);
	break;

      }
    } else {
      cprintf(ch, "I don't see that here.\n\r");
    }
  }
}

void do_stat(struct char_data *ch, char *argument, int cmd)
{
  struct affected_type *aff;
  char buf[MAX_STRING_LENGTH];
  char buf2[MAX_STRING_LENGTH];
  struct room_data *rm = 0;
  struct char_data *k = 0;
  struct obj_data *j = 0;
  struct obj_data *j2 = 0;
  struct extra_descr_data *desc;
  struct follow_type *fol;
  int i, virtual;
  int i2, count;
  BYTE found;
  struct room_data *rp = 0;
  char type[MAX_STRING_LENGTH], num[MAX_STRING_LENGTH];
  int number;

  if (IS_NPC(ch))
    return;

  bzero(type, MAX_STRING_LENGTH);
  bzero(num, MAX_STRING_LENGTH);
  argument = one_argument(argument, type);
  only_argument(argument, num);
  if(!*num)
    number= -2;
  else if(isdigit(*num))
    number= atoi(num);
  else
    number= -1;

  /* no argument */
  if (!*type) {
    cprintf(ch, "Usage: stat < pc|mob|obj|room > [ name|vnum ]\n\r");
    return;
  }
  /* ROOM  */
  if(!str_cmp("room", type) || !str_cmp("here", type)) {
    if(number < 0) {
      if(number == -2)
        number= ch->in_room;
      else {
        cprintf(ch, "Usage: stat room [vnum]\n\r");
        return;
      }
    }
    rm = real_roomp(number);
    cprintf(ch, "Room Description: ---------------------------------------------------------\n\r%s", rm->description);
    if((desc= rm->ex_description)) {
      cprintf(ch, "---------------------------------------------------------------------------\n\r");
      *buf= '\0';
      for(;desc;desc= desc->next) {
        strcat(buf, desc->keyword);
        strcat(buf, " ");
      }
      cprintf(ch, "Extras: %s\n\r", buf);
    }
    cprintf(ch, "---------------------------------------------------------------------------\n\r");
    sprinttype(rm->sector_type, sector_types, buf2);
    cprintf(ch, "%s [#%d], in Zone %s [#%d] is %s.\n\r",
            rm->name, rm->number, zone_table[rm->zone].name, rm->zone, buf2);
    if(rm->tele_targ > 0) { /* teleport room */
      double ttime= (double)rm->tele_time/(double)10.0;
      rp = real_roomp(rm->tele_targ);
      cprintf(ch, "Teleports to %s [#%d] every %3.1lf second%s",
        rp?rp->name:"Swirling CHAOS", rm->tele_targ,
        ttime, (ttime != 1.0)?"s.\n\r":".\n\r");
    }
    if((rm->sector_type == SECT_WATER_SWIM)||
       (rm->sector_type == SECT_WATER_NOSWIM)) {
      if(rm->river_dir != -1 && rm->dir_option[rm->river_dir]) {
        double ttime= (double)rm->river_speed/(double)10.0;
        rp= real_roomp(rm->dir_option[rm->river_dir]->to_room);
        cprintf(ch,
          "A River flows %s into %s [#%d] every %3.1lf second%s",
          dirs[rm->river_dir], rp?rp->name:"Swirling CHAOS", rp?rp->number:-1,
          ttime, (ttime != 1.0)?"s.\n\r":".\n\r");
      }
    }
    if(rm->room_flags) {
      sprintbit((long)rm->room_flags, room_bits, buf);
      cprintf(ch, "Flags: %s\n\r", buf);
    }
    if(rm->room_flags & SOUND) {
      cprintf(ch, "Sound: %s", rm->sound);
      cprintf(ch, "Sound: %s", rm->distant_sound);
    }
    if(rm->funct) {
      cprintf(ch, "Special Procedure: %s.\n\r",
              name_special_proc(SPECIAL_ROOM, rm->number));
    }
    for(i= 0; i< MAX_NUM_EXITS; i++) {
      if(rm->dir_option[i]) {
        rp= real_roomp(rm->dir_option[i]->to_room);
        cprintf(ch, "Exit %s to %s [#%d] is called %s.\n\r", dirs[i],
                rp?rp->name:"Swirling CHAOS", rp?rp->number:-1,
                rm->dir_option[i]->keyword?
                rm->dir_option[i]->keyword:dirs[i]);
        if (rm->dir_option[i]->general_description)
          cprintf(ch, "     %s",
                  rm->dir_option[i]->general_description);
        if(rm->dir_option[i]->exit_info) {
          sprintbit(rm->dir_option[i]->exit_info, exit_bits, buf2);
          cprintf(ch, "     Flags: %s\n\r", buf2);
        }
        if(rm->dir_option[i]->key > 0) {
          cprintf(ch, "     Key: %s [#%d]\n\r",
                obj_index[rm->dir_option[i]->key].name,
                obj_index[rm->dir_option[i]->key].virtual);
        }
      }
    }
    if((k= rm->people)) {
      cprintf(ch, "Lifeforms present:\n\r");
      for(; k; k = k->next_in_room) {
        if(CAN_SEE(ch, k)) {
          register int v;
          sprintf(buf, "%s", GET_NAME(k));
          if(!(v= MobVnum(k)))
            strcat(buf, "(PC)");
          else if(v < 0)
            strcat(buf, "(NPC)");
          else
            sprintf(buf+strlen(buf), " [#%d]", v);
          cprintf(ch, "     %s\n\r", buf);
        }
      }
    }
    if((j= rm->contents)) {
      cprintf(ch, "Objects present:\n\r");
      for(; j; j = j->next_content)
        cprintf(ch, "     %s [#%d]\n\r", j->name, ObjVnum(j));
    }
    return;
  } else if(!str_cmp("mob", type) || !str_cmp("pc", type)) {
    count = 1;

    k= NULL;
    if(number < 0) {
      if(number == -2)
        k= ch;
    }
    /* MOBILE in world */
    if(number >= 0) {
      if(!(k= get_char_num(number))) {
        cprintf(ch, "Noone with that vnum exists, I shall load one!\n\r");
        if(!(k= read_mobile(number, VIRTUAL))) {
          cprintf(ch, "No such creature exists in Reality!\n\r");
          return;
        } else {
          cprintf(ch, "%s appears for your inspection.\n\r", NAME(k));
          char_to_room(k, ch->in_room);
        }
      }
    } else if(!k) {
      if(!str_cmp("me", num)) {
        k= ch;
      } else if(!(k = get_char_room_vis(ch, num))) {
        if(!(k = get_char_vis_world(ch, num, &count))) {
          register int x;
          cprintf(ch, "No creature exists by that name, I shall make one!\n\r");
          for(x= 0; x< top_of_mobt; x++) {
            if(isname(num, mob_index[x].name)) {
              if(!(k= read_mobile(x, REAL))) {
                cprintf(ch, "No such creature exists in Reality!\n\r");
                return;
              } else {
                cprintf(ch, "%s appears for your inspection.\n\r", NAME(k));
                char_to_room(k, ch->in_room);
                x= -1;
                break;
              }
            }
          }
          if(x> -1) {
            cprintf(ch, "No such creature exists in Reality!\n\r");
            return;
          }
        }
      }
    }
    sprintf(buf2, "Name: %s  :  [R-Number %d]  ", GET_NAME(k), k->nr);
    if (IS_MOB(k)) {
	sprintf(buf2 + strlen(buf2),
		"[Load Number %d]\n\r", mob_index[k->nr].virtual);
    } else {
	strcat(buf2, "\n\r");
    }
    cprintf(ch, buf2);
    sprintf(buf2, "Location [%d]\n\r", k->in_room);

    switch (k->player.sex) {
    case SEX_NEUTRAL:
	strcpy(buf, "Neutral-Sex");
	break;
    case SEX_MALE:
	strcpy(buf, "Male");
	break;
    case SEX_FEMALE:
	strcpy(buf, "Female");
	break;
    default:
	strcpy(buf, "ILLEGAL-SEX!!");
	break;
    }

    sprintf(buf2 + strlen(buf2), "Sex : %s - %s\n\r",
	      buf,
	      (!IS_NPC(k) ? "Pc" : (!IS_MOB(k) ? "Npc" : "Mob")));

    cprintf(ch, buf2);

    strcpy(buf, "Short description: ");
    strcat(buf, (k->player.short_descr ? k->player.short_descr : "None"));
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    strcpy(buf, "Title: ");
    strcat(buf, (k->player.title ? k->player.title : "None"));
    strcat(buf, "\n\r");
    cprintf(ch, buf);
    strcpy(buf, "Pre-Title: ");
    strcat(buf, (GET_PRETITLE(k) ? GET_PRETITLE(k) : "None"));
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    cprintf(ch, "Long description: ");
    if (k->player.long_descr)
	cprintf(ch, k->player.long_descr);
    else
	cprintf(ch, "None");
    cprintf(ch, "\n\r");

    if (IS_NPC(k)) {
	strcpy(buf, "Monster Class: ");
	sprinttype(k->player.class, npc_class_types, buf2);
    } else {
	strcpy(buf, "Class: ");
	sprintbit(k->player.class, pc_class_types, buf2);
    }
    strcat(buf, buf2);

    sprintf(buf2, " :  Level [%d/%d/%d/%d/%d] : Alignment[%d]\n\r",
	      k->player.level[0], k->player.level[1],
	      k->player.level[2], k->player.level[3],
	      k->player.level[4], GET_ALIGNMENT(k));

    strcat(buf, buf2);
    cprintf(ch, buf);

    if (IS_PC(k)) {
	cprintf(ch, "Birth : [%ld] secs, Logon[%ld] secs, Played[%ld] secs\n\r",
		k->player.time.birth,
		k->player.time.logon,
		k->player.time.played);

	cprintf(ch, "Age: [%d] Years,  [%d] Months,  [%d] Days,  [%d] Hours\n\r",
		age(k).year, age(k).month, age(k).day, age(k).hours);
    }
    cprintf(ch, "Height [%d]cm  Weight [%d]pounds \n\r",
	      GET_HEIGHT(k), GET_WEIGHT(k));
    cprintf(ch, "+----------------------------+\n\r");
    cprintf(ch, "Str:[%d/%d]  Int:[%d]  Wis:[%d]  Dex:[%d]  Con:[%d]\n\r",
	      GET_STR(k), GET_ADD(k),
	      GET_INT(k),
	      GET_WIS(k),
	      GET_DEX(k),
	      GET_CON(k));

    cprintf(ch,
	      "Mana p.:[%d/%d+%d]  Hit p.:[%d/%d+%d]  Move p.:[%d/%d+%d]\n\r",
	      GET_MANA(k), mana_limit(k), mana_gain(k),
	      GET_HIT(k), hit_limit(k), hit_gain(k),
	      GET_MOVE(k), move_limit(k), move_gain(k));

    cprintf(ch,
	      "AC:[%d/10], Coins: [%d], Exp: [%d], Hitroll: [%d], Damroll: [%d]\n\r",
	      GET_AC(k),
	      GET_GOLD(k),
	      GET_EXP(k),
	      k->points.hitroll,
	      k->points.damroll);

    if (IS_NPC(k)) {
	cprintf(ch, "Npc Bare Hand Damage %dd%d.\n\r",
		k->specials.damnodice, k->specials.damsizedice);
    }
    if (IS_PC(k)) {
	cprintf(ch, "\n\rTimer [%d] \n\r", k->specials.timer);
    }
    cprintf(ch, "+----------------------------+\n\r");

    sprinttype(GET_POS(k), position_types, buf2);
    sprintf(buf, "Position: %s : Fighting: %s", buf2,
	      ((k->specials.fighting) ? GET_NAME(k->specials.fighting) : "Nobody"));
    if (k->desc) {
	sprinttype(k->desc->connected, connected_types, buf2);
	strcat(buf, " : Connected: ");
	strcat(buf, buf2);
    }
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    strcpy(buf, "Default position: ");
    sprinttype((k->specials.default_pos), position_types, buf2);
    strcat(buf, buf2);
    if (IS_NPC(k)) {
	strcat(buf, "\n\rNPC flags: ");
	sprintbit(k->specials.act, action_bits, buf2);
    } else {
	strcat(buf, ",PC flags: ");
	sprintbit(k->specials.act, player_bits, buf2);
    }

    strcat(buf, buf2);

    if (IS_MOB(k)) {
	/* strcpy(buf, "\n\rMobile Special procedure : ");
	   strcat(buf, (mob_index[k->nr].func ? "Exists\n\r" : "None\n\r"));
           cprintf(ch, buf);
         */
        cprintf(ch, "\n\rMobile Special procedure : %s\n\r",
	       (mob_index[k->nr].func ?
               MobFunctionNameByFunc(mob_index[k->nr].func) : "None"));
    }
    cprintf(ch, "Carried weight: %d   Carried items: %d\n\r",
	      IS_CARRYING_W(k),
	      IS_CARRYING_N(k));

    for (i = 0, j = k->carrying; j; j = j->next_content, i++);
    sprintf(buf, "Items in inventory: %d, ", i);

    for (i = 0, i2 = 0; i < MAX_WEAR; i++)
	if (k->equipment[i])
	  i2++;
    sprintf(buf2, "Items in equipment: %d\n\r", i2);
    strcat(buf, buf2);
    cprintf(ch, buf);

    cprintf(ch, "Apply saving throws: [%d] [%d] [%d] [%d] [%d]\n\r",
	      k->specials.apply_saving_throw[0],
	      k->specials.apply_saving_throw[1],
	      k->specials.apply_saving_throw[2],
	      k->specials.apply_saving_throw[3],
	      k->specials.apply_saving_throw[4]);

    if (IS_PC(k)) {
	cprintf(ch, "Thirst: %d, Hunger: %d, Drunk: %d\n\r",
		k->specials.conditions[THIRST],
		k->specials.conditions[FULL],
		k->specials.conditions[DRUNK]);
    }
    cprintf(ch, "Master is '%s'\n\r",
	      ((k->master) ? GET_NAME(k->master) : "NOBODY"));
    cprintf(ch, "Followers are:\n\r");
    for (fol = k->followers; fol; fol = fol->next)
	if (CAN_SEE(ch, fol->follower))
	  act("    $N", FALSE, ch, 0, fol->follower, TO_CHAR);

    /* immunities */
    cprintf(ch, "Immune to:");
    sprintbit(k->M_immune, immunity_names, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);
    /* resistances */
    cprintf(ch, "Resistant to:");
    sprintbit(k->immune, immunity_names, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);
    /* Susceptible */
    cprintf(ch, "Susceptible to:");
    sprintbit(k->susc, immunity_names, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    /* Showing the bitvector */
    sprintbit(k->specials.affected_by, affected_bits, buf);
    cprintf(ch, "Affected by: ");
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    /* Routine to show what spells a char is affected by */
    if (k->affected) {
	cprintf(ch, "\n\rAffecting Spells:\n\r--------------\n\r");
	for (aff = k->affected; aff; aff = aff->next) {
	  cprintf(ch, "Spell : '%s'\n\r", spell_info[aff->type].name);
	  cprintf(ch, "     Modifies %s by %d points\n\r", apply_types[(int)aff->location], aff->modifier);
	  cprintf(ch, "     Expires in %3d hours, Bits set ", aff->duration);
	  sprintbit(aff->bitvector, affected_bits, buf);
	  strcat(buf, "\n\r");
	  cprintf(ch, buf);
	}
    }
    return;
  } else if(!str_cmp("obj", type)) {
    count = 1;

    j= NULL;
    if(number == -2) {
      cprintf(ch, "Usage: stat obj <name|vnum>\n\r");
      return;
    }
    /* OBJECT in world */
    if(number >= 0) {
      if(!(j= get_obj_num(number))) {
        cprintf(ch, "Nothing with that vnum exists, I shall load one!\n\r");
        if(!(j= read_object(number, VIRTUAL))) {
          cprintf(ch, "No such object exists in Reality!\n\r");
          return;
        } else {
          cprintf(ch, "A new %s appears for your inspection.\n\r",
                  j->short_description);
          obj_to_room(j, ch->in_room);
        }
      }
    } else if(!j) {
      if(!(j = get_obj_vis(ch, num))) {
        cprintf(ch, "No such object is visible in the Realm.\n\r");
        return;
      }
    }
    virtual = (j->item_number >= 0) ? obj_index[j->item_number].virtual : 0;
    sprintf(buf, "Object name: [%s]\n\rR-number: [%d], Load Number: [%d]\n\rItem type: ",
	      j->name, j->item_number, virtual);
    sprinttype(GET_ITEM_TYPE(j), item_types, buf2);
    strcat(buf, buf2);
    strcat(buf, "\n\r");
    cprintf(ch, buf);
    cprintf(ch, "Short description: %s\n\rLong description:\n\r%s\n\r",
	   ((j->short_description) ? j->short_description : "None"),
	      ((j->description) ? j->description : "None"));
    if (j->ex_description) {
	strcpy(buf, "Extra description keyword(s):\n\r----------\n\r");
	for (desc = j->ex_description; desc; desc = desc->next) {
	  strcat(buf, desc->keyword);
	  strcat(buf, "\n\r");
	}
	strcat(buf, "----------\n\r");
	cprintf(ch, buf);
    } else {
	strcpy(buf, "Extra description keyword(s): None\n\r");
	cprintf(ch, buf);
    }

    cprintf(ch, "Can be worn on :");
    sprintbit(j->obj_flags.wear_flags, wear_bits, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    cprintf(ch, "Set char bits  :");
    sprintbit(j->obj_flags.bitvector, affected_bits, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    cprintf(ch, "Extra flags: ");
    sprintbit(j->obj_flags.extra_flags, extra_bits, buf);
    strcat(buf, "\n\r");
    cprintf(ch, buf);

    cprintf(ch, "Weight: %d, Value: %d, Cost/day: %d, Timer: %d\n\r",
	      j->obj_flags.weight, j->obj_flags.cost,
	      j->obj_flags.cost_per_day, j->obj_flags.timer);

    strcpy(buf, "In room: ");
    if (j->in_room == NOWHERE)
	strcat(buf, "Nowhere");
    else {
	sprintf(buf2, "%d", j->in_room);
	strcat(buf, buf2);
    }
    strcat(buf, " ,In object: ");
    strcat(buf, (!j->in_obj ? "None" : fname(j->in_obj->name)));

    /*
     * strcat(buf," ,Carried by:");
     * if (j->carried_by) 
     * {
     * if (GET_NAME(j->carried_by)) 
     * {
     * if (strlen(GET_NAME(j->carried_by)) > 0) 
     * {
     * strcat(buf, (!j->carried_by) ? "Nobody" : GET_NAME(j->carried_by));
     * }
     * else
     * {
     * strcat(buf, "NonExistantPlayer");
     * }
     * }
     * else 
     * {
     * strcat(buf, "NonExistantPlayer");
     * }
     * }
     * else 
     * {
     * strcat(buf, "Nobody");
     * }
     * strcat(buf,"\n\r");
     * cprintf(ch, buf);
     */
    switch (j->obj_flags.type_flag) {
    case ITEM_LIGHT:
	sprintf(buf, "Colour : [%d]\n\rType : [%d]\n\rHours : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2]);
	break;
    case ITEM_SCROLL:
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    case ITEM_WAND:
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
    case ITEM_STAFF:
	sprintf(buf, "Spell : %d\n\rMana : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);
	break;
    case ITEM_WEAPON:
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    case ITEM_FIREWEAPON:
	sprintf(buf, "Tohit : %d\n\rTodam : %dD%d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    case ITEM_MISSILE:
	sprintf(buf, "Tohit : %d\n\rTodam : %d\n\rType : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3]);
	break;
    case ITEM_ARMOR:
	sprintf(buf, "AC-apply : [%d]\n\rFull Strength : [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1]);

	break;
    case ITEM_POTION:
	sprintf(buf, "Spells : %d, %d, %d, %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    case ITEM_TRAP:
	sprintf(buf, "level: %d, att type: %d, damage class: %d, charges: %d",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    case ITEM_CONTAINER:
	sprintf(buf, "Max-contains : %d\n\rLocktype : %d\n\rCorpse : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3] ? "Yes" : "No");
	break;
    case ITEM_DRINKCON:
	sprinttype(j->obj_flags.value[2], drinks, buf2);
	sprintf(buf, "Max-contains : %d\n\rContains : %d\n\rPoisoned : %d\n\rLiquid : %s",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[3],
		buf2);
	break;
    case ITEM_NOTE:
	sprintf(buf, "Tounge : %d",
		j->obj_flags.value[0]);
	break;
    case ITEM_KEY:
	sprintf(buf, "Keytype : %d",
		j->obj_flags.value[0]);
	break;
    case ITEM_FOOD:
	sprintf(buf, "Makes full : %d\n\rPoisoned : %d",
		j->obj_flags.value[0],
		j->obj_flags.value[3]);
	break;
    default:
	sprintf(buf, "Values 0-3 : [%d] [%d] [%d] [%d]",
		j->obj_flags.value[0],
		j->obj_flags.value[1],
		j->obj_flags.value[2],
		j->obj_flags.value[3]);
	break;
    }
    cprintf(ch, buf);

    strcpy(buf, "\n\rEquipment Status: ");
    if (!j->carried_by)
	strcat(buf, "NONE");
    else {
	found = FALSE;
	for (i = 0; i < MAX_WEAR; i++) {
	  if (j->carried_by->equipment[i] == j) {
	    sprinttype(i, equipment_types, buf2);
	    strcat(buf, buf2);
	    found = TRUE;
	  }
	}
	if (!found)
	  strcat(buf, "Inventory");
    }
    cprintf(ch, buf);

    strcpy(buf, "\n\rSpecial procedure : ");
    if (j->item_number >= 0)
	strcat(buf, (obj_index[j->item_number].func ? "exists\n\r" : "No\n\r"));
    else
	strcat(buf, "No\n\r");
    cprintf(ch, buf);

    strcpy(buf, "Contains :\n\r");
    found = FALSE;
    for (j2 = j->contains; j2; j2 = j2->next_content) {
	strcat(buf, fname(j2->name));
	strcat(buf, "\n\r");
	found = TRUE;
    }
    if (!found)
	strcpy(buf, "Contains : Nothing\n\r");
    cprintf(ch, buf);

    cprintf(ch, "Can affect char :\n\r");
    for (i = 0; i < MAX_OBJ_AFFECT; i++) {
	sprinttype(j->affected[i].location, apply_types, buf2);
	cprintf(ch, "    Affects : %s By %d\n\r", buf2, j->affected[i].modifier);
    }
    return;
  } else {
    cprintf(ch, "Usage: stat < pc|mob|obj|room > [ name|vnum ]\n\r");
    return;
  }
}

void do_pretitle(struct char_data *ch, char *argument, int cmd)
{
  char name[20];
  char pretitle[50];
  struct char_data *vict;

  argument = one_argument(argument, name);
  if(*argument == ' ') argument++;
  strcpy(pretitle, argument);

  if ((vict = get_char_vis(ch, name)) == NULL) {
    cprintf(ch, "I don't see them here?\n\r");
    return;
  }
  if ((strlen(pretitle) == 0)) {
    GET_PRETITLE(vict) = 0;
    return;
  }
  GET_PRETITLE(vict) = (char *)calloc(1, strlen(pretitle)+1);
  strcpy(GET_PRETITLE(vict), pretitle);
}

void do_set(struct char_data *ch, char *argument, int cmd)
{
  char field[20], name[20], parmstr[50];
  int index;
  struct char_data *mob;
  int parm;
  char *pset_list[] =
  {
    "align", "exp", "sex", "race", "tohit", "dmg",
    "bank", "gold", "prac",
    "str", "int", "wis", "dex", "con", "stradd",
    "hit", "mhit", "mana", "mmana", "move", "mmove",
    "mlvl","clvl","wlvl","tlvl","rlvl","dlvl",
    "aggr", "wander",
    NULL
  };

  char tmp[80];
  char buf[MAX_STRING_LENGTH];
  int i, no;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, name);
  argument = one_argument(argument, field);
  strncpy(tmp, argument, 79);
  argument = one_argument(argument, parmstr);

  if ((mob = get_char_vis(ch, name)) == NULL) {
    cprintf(ch, "I don't see them here? \n\r\n\r");
    *buf = '\0';
    strcpy(buf, "Usage:  pset <name> <attrib> <value>\n\r");
    for (no = 1, i = 0; pset_list[i]; i++) {
      sprintf(buf + strlen(buf), "%-10s", pset_list[i]);
      if (!(no % 7))
        strcat(buf, "\n\r");
      no++;
    }
    strcat(buf, "\n\r");
    cprintf(ch, buf);
    return;
  }
  for (index = 0; pset_list[index]; index++)
    if (!strcmp(field, pset_list[index])) {
      int x;
      x= sscanf(parmstr, "%d", &parm);
      if (!x) {
	cprintf(ch, "You must also supply a value\n\r");
	return;
      }
      break;
    }
  if(IS_PC(mob) && mob != ch && GetMaxLevel(mob) >= GetMaxLevel(ch)) {
    cprintf(ch, "You wish you could set %s's stats...\n\r", GET_NAME(mob));
    return;
  }
  switch (index) {
  case 0:
    GET_ALIGNMENT(mob) = parm;
    break;
  case 1:
    GET_EXP(mob) = parm;
    break;
  case 2:
    GET_SEX(mob) = parm;
    break;
  case 3:
    GET_RACE(mob) = parm;
    break;
  case 4:
    GET_HITROLL(mob) = parm;
    break;
  case 5:
    GET_DAMROLL(mob) = parm;
    break;
  case 6:
    GET_BANK(mob) = parm;
    break;
  case 7:
    GET_GOLD(mob) = parm;
    break;
  case 8:
    mob->specials.pracs = parm;
    break;
  case 9:
    if(ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    mob->abilities.str = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 10:
    if(ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    mob->abilities.intel = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 11:
    if(ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    mob->abilities.wis = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 12:
    if(ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    mob->abilities.dex = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 13:
    if(ch == mob && parm > 25 && GetMaxLevel(ch) < LOKI) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    mob->abilities.con = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 14:
    mob->abilities.str_add = parm;
    mob->tmpabilities = mob->abilities;
    break;
  case 15:
    GET_HIT(mob) = parm;
    break;
  case 16:
    mob->points.max_hit = parm;
    break;
  case 17:
    GET_MANA(mob) = parm;
    break;
  case 18:
    mob->points.max_mana = parm;
    break;
  case 19:
    GET_MOVE(mob) = parm;
    break;
  case 20:
    mob->points.max_move = parm;
    break;
  case 21:
    if(ch == mob && parm > GET_LEVEL(ch, MAGE_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, MAGE_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_MAGIC_USER;
    } else {
      GET_CLASS(mob) |= CLASS_MAGIC_USER;
    }
    GET_LEVEL(mob, MAGE_LEVEL_IND)= parm;
    break;
  case 22:
    if(ch == mob && parm > GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, CLERIC_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_CLERIC;
    } else {
      GET_CLASS(mob) |= CLASS_CLERIC;
    }
    GET_LEVEL(mob, CLERIC_LEVEL_IND)= parm;
    break;
  case 23:
    if(ch == mob && parm > GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, WARRIOR_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_WARRIOR;
    } else {
      GET_CLASS(mob) |= CLASS_WARRIOR;
    }
    GET_LEVEL(mob, WARRIOR_LEVEL_IND)= parm;
    break;
  case 24:
    if(ch == mob && parm > GET_LEVEL(ch, THIEF_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, THIEF_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_THIEF;
    } else {
      GET_CLASS(mob) |= CLASS_THIEF;
    }
    GET_LEVEL(mob, THIEF_LEVEL_IND)= parm;
    break;
  case 25:
    if(ch == mob && parm > GET_LEVEL(ch, RANGER_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, RANGER_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_RANGER;
    } else {
      GET_CLASS(mob) |= CLASS_RANGER;
    }
    GET_LEVEL(mob, RANGER_LEVEL_IND)= parm;
    break;
  case 26:
    if(ch == mob && parm > GET_LEVEL(ch, DRUID_LEVEL_IND)) {
      cprintf(ch, "Sure, we all want to be more powerful.\n\r");
      return;
    }
    if(ch != mob && IS_IMMORTAL(mob) && str_cmp(GET_NAME(ch), "Quixadhal")
       && parm > GET_LEVEL(ch, DRUID_LEVEL_IND)) {
      cprintf(ch, "Ask the Dread Lord to make %s mightier!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm < 1) {
      GET_CLASS(mob) &= ~CLASS_DRUID;
    } else {
      GET_CLASS(mob) |= CLASS_DRUID;
    }
    GET_LEVEL(mob, DRUID_LEVEL_IND)= parm;
    break;
  case 27:
    if(!IS_NPC(mob)) {
      cprintf(ch, "You should tell %s to be more aggressive!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm) {
      if (!IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        SET_BIT(mob->specials.act, ACT_AGGRESSIVE);
        cprintf(ch, "%s is now AGGRESSIVE!\n\r", NAME(mob));
      }
    } else {
      if (IS_SET(mob->specials.act, ACT_AGGRESSIVE)) {
        REMOVE_BIT(mob->specials.act, ACT_AGGRESSIVE);
        cprintf(ch, "%s is now nice.\n\r", NAME(mob));
      }
    }
    break;
  case 28:
    if(!IS_NPC(mob)) {
      cprintf(ch, "You should tell %s to wander about more!\n\r",
              GET_NAME(mob));
      return;
    }
    if(parm) {
      if (IS_SET(mob->specials.act, ACT_SENTINEL)) {
        REMOVE_BIT(mob->specials.act, ACT_SENTINEL);
        cprintf(ch, "%s is now wandering!\n\r", NAME(mob));
      }
    } else {
      if (!IS_SET(mob->specials.act, ACT_SENTINEL)) {
        SET_BIT(mob->specials.act, ACT_SENTINEL);
        cprintf(ch, "%s is now lazy.\n\r", NAME(mob));
      }
    }
    break;
  default:
    *buf = '\0';
    strcpy(buf, "Usage:  pset <name> <attrib> <value>\n\r");
    for (no = 1, i = 0; pset_list[i]; i++) {
      sprintf(buf + strlen(buf), "%-10s", pset_list[i]);
      if (!(no % 7))
        strcat(buf, "\n\r");
      no++;
    }
    strcat(buf, "\n\r");
    cprintf(ch, buf);
  }
}

void do_shutdow(struct char_data *ch, char *argument, int cmd)
{
  cprintf(ch, "If you want to shut something down - say so!\n\r");
}

void do_shutdown(struct char_data *ch, char *argument, int cmd)
{
  long tc;
  struct tm *t_info;
  char *tmstr;
  char buf[100], arg[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  tc = time(0);
  t_info = localtime(&tc);
  tmstr= asctime(t_info);
  *(tmstr + strlen(tmstr) -1) = '\0';

  one_argument(argument, arg);

  if (!*arg) {
    sprintf(buf, "SHUTDOWN by %s at %d:%d", GET_NAME(ch),
            t_info->tm_hour + 1, t_info->tm_min);
    log(buf);
    aprintf(
      "\x007\n\rBroadcast message from %s (tty0) %s...\n\r\n\r",
      GET_NAME(ch), tmstr);
    aprintf("\x007The system is going down NOW !!\n\r\x007\n\r");
    diku_shutdown = 1;
    update_time_and_weather();
  } else if (!str_cmp(arg, "-k")) {
    sprintf(buf, "FAKE REBOOT by %s at %d:%d", GET_NAME(ch),
            t_info->tm_hour + 1, t_info->tm_min);
    log(buf);
    aprintf(
      "\x007\n\rBroadcast message from %s (tty0) %s...\n\r\n\r",
      GET_NAME(ch), tmstr);
    aprintf("\x007Rebooting.  Come back in a few minutes!\n\r");
    aprintf("\x007The system is going down NOW !!\n\r\n\r");
  } else if (!str_cmp(arg, "-r")) {
    sprintf(buf, "REBOOT by %s at %d:%d", GET_NAME(ch),
            t_info->tm_hour + 1, t_info->tm_min);
    log(buf);
    aprintf(
      "\x007\n\rBroadcast message from %s (tty0) %s...\n\r\n\r",
      GET_NAME(ch), tmstr);
    aprintf("\x007Rebooting.  Come back in a few minutes!\n\r");
    aprintf("\x007The system is going down NOW !!\n\r\n\r");
    diku_shutdown = diku_reboot = 1;
    update_time_and_weather();
  } else
    cprintf(ch, "Go shut down someone your own size.\n\r");
}

void do_snoop(struct char_data *ch, char *argument, int cmd)
{
  static char arg[MAX_STRING_LENGTH];
  struct char_data *victim;

  if (!ch->desc)
    return;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if (!*arg) {
    cprintf(ch, "Snoop who ?\n\r");
    return;
  }
  if (!(victim = get_char_vis(ch, arg))) {
    cprintf(ch, "No such person around.\n\r");
    return;
  }
  if (!victim->desc) {
    cprintf(ch, "There's no link.. nothing to snoop.\n\r");
    return;
  }
  if (victim == ch) {
    cprintf(ch, "Ok, you just snoop yourself.\n\r");
    if (ch->desc->snoop.snooping) {
      if (ch->desc->snoop.snooping->desc)
	ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;
      else {
	char buf[MAX_STRING_LENGTH];

	sprintf(buf, "caught %s snooping %s who didn't have a descriptor!",
	    ch->player.name, ch->desc->snoop.snooping->player.name);
	log(buf);
      }
      ch->desc->snoop.snooping = 0;
    }
    return;
  }
  if (victim->desc->snoop.snoop_by) {
    cprintf(ch, "Busy already. \n\r");
    return;
  }
  if (GetMaxLevel(victim) >= GetMaxLevel(ch)) {
    cprintf(ch, "You failed.\n\r");
    return;
  }
  cprintf(ch, "Ok. \n\r");

  if (ch->desc->snoop.snooping)
    if (ch->desc->snoop.snooping->desc)
      ch->desc->snoop.snooping->desc->snoop.snoop_by = 0;

  ch->desc->snoop.snooping = victim;
  victim->desc->snoop.snoop_by = ch;
  return;
}

void do_switch(struct char_data *ch, char *argument, int cmd)
{
  static char arg[80];
  struct char_data *victim;

  if (IS_NPC(ch))
    return;

  only_argument(argument, arg);

  if (!*arg) {
    cprintf(ch, "Switch with who?\n\r");
  } else {
    if(!(victim = get_char_room_vis(ch, arg))) {
      if (!(victim = get_char(arg))) {
        cprintf(ch, "They aren't here.\n\r");
        return;
      }
    } {
      if (ch == victim) {
	cprintf(ch, "He he he... We are jolly funny today, eh?\n\r");
	return;
      }
      if (!ch->desc || ch->desc->snoop.snoop_by || ch->desc->snoop.snooping) {
	cprintf(ch, "Mixing snoop & switch is bad for your health.\n\r");
	return;
      }
      if (victim->desc || (!IS_NPC(victim)) || IS_SET(victim->specials.act, ACT_SWITCH)) {
	cprintf(ch, "You can't do that, the body is already in use!\n\r");
      } else {
	cprintf(ch, "Ok.\n\r");

	ch->desc->character = victim;
	ch->desc->original = ch;

        SET_BIT(victim->specials.act, ACT_SWITCH);
	victim->desc = ch->desc;
	ch->desc = 0;
      }
    }
  }
}

void do_return(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *mob = NULL, *per = NULL;

  if (!ch->desc)
    return;

  if (!ch->desc->original || (IS_NOT_SET(ch->specials.act, ACT_SWITCH) &&
      IS_NOT_SET(ch->specials.act, ACT_POLYSELF) &&
      IS_NOT_SET(ch->specials.act, ACT_POLYOTHER))) {
    cprintf(ch, "Huh?  Talk sense I can't understand you.\n\r");
    return;
  } else {
    cprintf(ch, "You return to your original body.\n\r");

    if ((IS_SET(ch->specials.act, ACT_POLYSELF) ||
         IS_SET(ch->specials.act, ACT_POLYOTHER)) && cmd) {
      mob = ch;
      per = ch->desc->original;

      act("$n turns liquid, and reforms as $N", TRUE, mob, 0, per, TO_ROOM);

      char_from_room(per);
      char_to_room(per, mob->in_room);

      /*  SwitchStuff(mob, per); */
    }
    if(IS_SET(ch->specials.act, ACT_SWITCH))
      REMOVE_BIT(ch->specials.act, ACT_SWITCH);
    ch->desc->character = ch->desc->original;
    ch->desc->original = 0;

    ch->desc->character->desc = ch->desc;
    ch->desc = 0;

    if ((IS_SET(ch->specials.act, ACT_POLYSELF) ||
         IS_SET(ch->specials.act, ACT_POLYOTHER)) && cmd) {
      extract_char(mob);
    }
  }
}

void do_force(struct char_data *ch, char *argument, int cmd)
{
  struct descriptor_data *i;
  struct char_data *vict;
  char name[100], to_force[100], buf[100];

  if (IS_NPC(ch) && (cmd != 0))
    return;

  half_chop(argument, name, to_force);

  if (!*name || !*to_force)
    cprintf(ch, "Who do you wish to force to do what?\n\r");
  else if (str_cmp("all", name)) {
    if (!(vict = get_char_vis(ch, name)))
      cprintf(ch, "No-one by that name here..\n\r");
    else {
      if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) && (!IS_NPC(vict)))
	cprintf(ch, "Oh no you don't!!\n\r");
      else {
        if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
	  sprintf(buf, "$n has forced you to '%s'.", to_force);
	  act(buf, FALSE, ch, 0, vict, TO_VICT);
        }
	cprintf(ch, "Ok.\n\r");
	command_interpreter(vict, to_force);
      }
    }
  } else {			       /* force all */
    for (i = descriptor_list; i; i = i->next)
      if (i->character != ch && !i->connected && i->character != board_kludge_char) {
	vict = i->character;
	if ((GetMaxLevel(ch) <= GetMaxLevel(vict)) &&
	    (!IS_NPC(vict)))
	  cprintf(ch, "Oh no you don't!!\n\r");
	else {
          if (!IS_SET(ch->specials.act, PLR_STEALTH)) {
	    sprintf(buf, "$n has forced you to '%s'.", to_force);
	    act(buf, FALSE, ch, 0, vict, TO_VICT);
          }
	  command_interpreter(vict, to_force);
	}
      }
    cprintf(ch, "Ok.\n\r");
  }
}

void do_load(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *mob;
  struct obj_data *obj;
  char type[100], num[100];
  int number;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, type);

  only_argument(argument, num);
  if (isdigit(*num))
    number = atoi(num);
  else
    number = -1;

  if (is_abbrev(type, "mobile")) {
    if (number < 0) {
      for (number = 0; number <= top_of_mobt; number++)
	if (isname(num, mob_index[number].name))
	  break;
      if (number > top_of_mobt)
	number = -1;
    } else {
      number = real_mobile(number);
    }
    if (number < 0 || number > top_of_mobt) {
      cprintf(ch, "There is no such monster.\n\r");
      return;
    }
    mob = read_mobile(number, REAL);
    char_to_room(mob, ch->in_room);

    act("$n makes a quaint, magical gesture with one hand.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has summoned $N from the ether!", FALSE, ch, 0, mob, TO_ROOM);
    act("You bring forth $N from the the cosmic ether.", FALSE, ch, 0, mob, TO_CHAR);
  } else if (is_abbrev(type, "object")) {
    if (number < 0) {
      for (number = 0; number <= top_of_objt; number++)
	if (isname(num, obj_index[number].name))
	  break;
      if (number > top_of_objt)
	number = -1;
    } else {
      number = real_object(number);
    }
    if (number < 0 || number > top_of_objt) {
      cprintf(ch, "There is no such object.\n\r");
      return;
    }
    obj = read_object(number, REAL);
    obj_to_char(obj, ch);
    act("$n makes a strange magical gesture.", TRUE, ch, 0, 0, TO_ROOM);
    act("$n has created $p!", FALSE, ch, obj, 0, TO_ROOM);
    act("You now have $p.", FALSE, ch, obj, 0, TO_CHAR);
  } else {
    cprintf(ch, "Usage:  load <object|mobile> <vnum|name>\n\r");
  }
}

static void purge_one_room(int rnum, struct room_data *rp, int *range)
{
  struct char_data *ch;
  struct obj_data *obj;

  if (rnum == 0 || rnum < range[0] || rnum > range[1])
    return;

  while (rp->people) {
    ch = rp->people;
    cprintf(ch, "The gods strike down from the heavens making the");
    cprintf(ch, "world tremble.  All that's left is the Void.");
    char_from_room(ch);
    char_to_room(ch, 0);	       /* send character to the void */
    do_look(ch, "", 15);
    act("$n tumbles into the Void.", TRUE, ch, 0, 0, TO_ROOM);
  }

  while (rp->contents) {
    obj = rp->contents;
    obj_from_room(obj);
    obj_to_room(obj, 0);	       /* send item to the void */
  }
  completely_cleanout_room(rp);	       /* clear out the pointers */
  hash_remove(&room_db, rnum);	       /* remove it from the database */
}

/* clean a room of all mobiles and objects */
void do_purge(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  char name[100];

  if (IS_NPC(ch))
    return;

  only_argument(argument, name);

  if (*name) {			       /* argument supplied. destroy single object or char */
    if ((vict = get_char_room_vis(ch, name))) {
      if ((!IS_NPC(vict) || IS_SET(vict->specials.act, ACT_POLYSELF)) &&
	  (GetMaxLevel(ch) < IMPLEMENTOR)) {
	cprintf(ch, "I'm sorry, Dave.  I can't let you do that.\n\r");
	return;
      }
      act("$n disintegrates $N.", FALSE, ch, 0, vict, TO_NOTVICT);

      if (IS_NPC(vict) || (!IS_SET(ch->specials.act, ACT_POLYSELF))) {
	extract_char(vict);
      } else {
	if (vict->desc) {
	  close_socket(vict->desc);
	  vict->desc = 0;
	  extract_char(vict);
	} else {
	  extract_char(vict);
	}
      }
    } else if ((obj = get_obj_in_list_vis(ch, name, 
                       real_roomp(ch->in_room)->contents))) {
      act("$n destroys $p.", FALSE, ch, obj, 0, TO_ROOM);
      extract_obj(obj);
    } else {
      argument = one_argument(argument, name);
      if (0 == str_cmp("room", name)) {
	int range[2];

	if (GetMaxLevel(ch) < IMPLEMENTOR) {
	  cprintf(ch, "I'm sorry, Dave.  I can't let you do that.\n\r");
	  return;
	}
	argument = one_argument(argument, name);
	if (!isdigit(*name)) {
	  cprintf(ch, "purge room start [end]");
	  return;
	}
	range[0] = atoi(name);
	argument = one_argument(argument, name);
	if (isdigit(*name))
	  range[1] = atoi(name);
	else
	  range[1] = range[0];

	if (range[0] == 0 || range[1] == 0) {
	  cprintf(ch, "usage: purge room start [end]\n\r");
	  return;
	}
	hash_iterate(&room_db, (funcp)purge_one_room, range);
      } else {
	cprintf(ch, "I don't see that here.\n\r");
	return;
      }
    }

    cprintf(ch, "Ok.\n\r");
  } else {			       /* no argument. clean out the room */
    if (GetMaxLevel(ch) < DEMIGOD)
      return;
    if (IS_NPC(ch)) {
      cprintf(ch, "You would only kill yourself..\n\r");
      return;
    }
    act("$n gestures, the world erupts around you in flames!",
	FALSE, ch, 0, 0, TO_ROOM);
    rprintf(ch->in_room, "The world seems a little cleaner.\n\r");

    for (vict = real_roomp(ch->in_room)->people; vict; vict = next_v) {
      next_v = vict->next_in_room;
      if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
	extract_char(vict);
    }

    for (obj = real_roomp(ch->in_room)->contents; obj; obj = next_o) {
      next_o = obj->next_content;
      extract_obj(obj);
    }
  }
}

/* Give pointers to the five abilities */
void roll_abilities(struct char_data *ch)
{
  int i, j, k, temp;
  UBYTE table[5];
  UBYTE rools[4];

  for (i = 0; i < 5; table[i++] = 0);

  for (i = 0; i < 5; i++) {

    for (j = 0; j < 4; j++)
      rools[j] = number(1, 6);

    temp = rools[0] + rools[1] + rools[2] + rools[3] -
      MIN(rools[0], MIN(rools[1], MIN(rools[2], rools[3])));

    for (k = 0; k < 5; k++)
      if (table[k] < temp)
	SWITCH(temp, table[k]);
  }

  ch->abilities.str_add = 0;

  switch (ch->player.class) {
  case CLASS_MAGIC_USER:{
      ch->abilities.intel = table[0];
      ch->abilities.wis = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_DRUID:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_CLERIC + CLASS_MAGIC_USER:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_THIEF:{
      ch->abilities.dex = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.str = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_MAGIC_USER:{
      ch->abilities.intel = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.str = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_THIEF + CLASS_MAGIC_USER + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.str = table[3];
      ch->abilities.con = table[4];
    }
    break;
  case CLASS_RANGER:{
      ch->abilities.str = table[0];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[2];
      ch->abilities.wis = table[1];
      ch->abilities.intel = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = number(0, 100);
    }
    break;
  case CLASS_WARRIOR:{
      ch->abilities.str = table[0];
      ch->abilities.con = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.wis = table[3];
      ch->abilities.intel = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = number(0, 100);
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER:{
      ch->abilities.str = table[0];
      ch->abilities.intel = table[1];
      ch->abilities.con = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.con = table[3];
      ch->abilities.dex = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_THIEF:{
      ch->abilities.str = table[0];
      ch->abilities.dex = table[1];
      ch->abilities.con = table[2];
      ch->abilities.intel = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_CLERIC:{
      ch->abilities.wis = table[0];
      ch->abilities.str = table[1];
      ch->abilities.intel = table[2];
      ch->abilities.dex = table[3];
      ch->abilities.con = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_MAGIC_USER + CLASS_THIEF:{
      ch->abilities.intel = table[0];
      ch->abilities.str = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.con = table[3];
      ch->abilities.wis = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  case CLASS_WARRIOR + CLASS_THIEF + CLASS_CLERIC:{
      ch->abilities.str = table[0];
      ch->abilities.wis = table[1];
      ch->abilities.dex = table[2];
      ch->abilities.intel = table[3];
      ch->abilities.con = table[4];
      if (ch->abilities.str == 18)
	ch->abilities.str_add = 0;
    }
    break;
  default:
    log("Error on class (do_reroll)");
  }

  switch (GET_RACE(ch)) {
  case RACE_ELVEN:
    ch->abilities.dex += 1;
    ch->abilities.con -= 1;
    break;
  case RACE_DWARF:
    ch->abilities.con += 1;
    ch->abilities.intel -= 1;
    break;
  case RACE_HALFLING:
    ch->abilities.dex += 1;
    ch->abilities.str -= 1;
    break;
  case RACE_GNOME:
    ch->abilities.intel += 1;
    ch->abilities.wis -= 1;
    break;
  }

  if (ch->abilities.str > 18)
    ch->abilities.str = 18;
  if (ch->abilities.dex > 18)
    ch->abilities.dex = 18;
  if (ch->abilities.intel > 18)
    ch->abilities.intel = 18;
  if (ch->abilities.wis > 18)
    ch->abilities.wis = 18;
  if (ch->abilities.con > 18)
    ch->abilities.con = 18;

  ch->tmpabilities = ch->abilities;
}

void do_start(struct char_data *ch)
{
  int r_num;
  struct obj_data *obj;

  void advance_level(struct char_data *ch, int i);

  StartLevels(ch);
  GET_EXP(ch) = 1;
  set_title(ch);
  roll_abilities(ch);

  ch->points.max_hit = 20;

/* Heafty Bread */
  if ((r_num = real_object(5016)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);	       /* */
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/* Bottle of Water */
  if ((r_num = real_object(3003)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/* Club */
  if ((r_num = real_object(3048)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/* Map of Shylar */
  if ((r_num = real_object(3050)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
/* Newbie note: added 9-25-95 by Sedna */
  if ((r_num = real_object(3105)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj,ch);
  }
/* Torch */
  if ((r_num = real_object(3015)) >= 0) {
    obj = read_object(r_num, REAL);
    obj_to_char(obj, ch);
  }
  if (IS_SET(ch->player.class, CLASS_RANGER)) {
    ch->skills[SKILL_TRACK].learned = 13;
    ch->skills[SKILL_DISARM].learned = 7;
  }
  if (IS_SET(ch->player.class, CLASS_THIEF)) {
    ch->skills[SKILL_SNEAK].learned = 1;
    ch->skills[SKILL_HIDE].learned = 13;
    ch->skills[SKILL_STEAL].learned = 7;
    ch->skills[SKILL_BACKSTAB].learned = 1;
    ch->skills[SKILL_PICK_LOCK].learned = 1;
  }
  GET_HIT(ch) = GET_MAX_HIT(ch);
  GET_MANA(ch) = GET_MAX_MANA(ch);
  ch->points.max_move = GET_MAX_MOVE(ch);
  GET_MOVE(ch) = GET_MAX_MOVE(ch);

  GET_COND(ch, THIRST) = 24;
  GET_COND(ch, FULL) = 24;
  GET_COND(ch, DRUNK) = 0;

  ch->player.time.played = 0;
  ch->player.time.logon = time(0);
}

void do_advance(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char name[100], level[100], class[100];
  int adv, newlevel, lin_class;

  void gain_exp(struct char_data *ch, int gain);

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, name);

  if (*name) {
    if (!(victim = get_char_room_vis(ch, name))) {
      cprintf(ch, "That player is not here.\n\r");
      return;
    }
  } else {
    cprintf(ch, "Advance who?\n\r");
    return;
  }

  if (IS_NPC(victim)) {
    cprintf(ch, "NO! Not on NPC's.\n\r");
    return;
  }
  if (IS_IMMORTAL(victim)) {
    cprintf(ch, "But they are already as powerful as you can imagine!\n\r");
    return;
  }
  argument = one_argument(argument, class);

  if (!*class) {
    cprintf(ch, "Classes you may suply: [ M C W T R ]\n\r");
    return;
  }
  switch (*class) {
  case 'M':
  case 'm':
    lin_class = MAGE_LEVEL_IND;
    break;

  case 'T':
  case 't':
    lin_class = THIEF_LEVEL_IND;
    break;

  case 'W':
  case 'w':
  case 'F':
  case 'f':
    lin_class = WARRIOR_LEVEL_IND;
    break;

  case 'C':
  case 'c':
  case 'P':
  case 'p':
    lin_class = CLERIC_LEVEL_IND;
    break;

  case 'R':
  case 'r':
    lin_class = RANGER_LEVEL_IND;
    break;

  case 'D':
  case 'd':
    lin_class = DRUID_LEVEL_IND;
    break;

  default:
    cprintf(ch, "Classes you may use [ M C W T R ]\n\r");
    return;
    break;

  }

  argument = one_argument(argument, level);

  if (GET_LEVEL(victim, lin_class) == 0)
    adv = 1;
  else if (!*level) {
    cprintf(ch, "You must supply a level number.\n\r");
    return;
  } else {
    if (!isdigit(*level)) {
      cprintf(ch, "Third argument must be a positive integer.\n\r");
      return;
    }
    if ((newlevel = atoi(level)) < GET_LEVEL(victim, lin_class)) {
      int i;
      if((i= GET_LEVEL(victim, lin_class)- newlevel) < 1) {
        cprintf(ch, "Sorry, must leave them at level 1 at least!\n\r");
        return;
      }
      for(;i> 0; i--)
        drop_level(victim, lin_class);
      set_title(victim);
      return;
    }
    adv = newlevel - GET_LEVEL(victim, lin_class);
  }

  if (((adv + GET_LEVEL(victim, lin_class)) > 1) &&
      (GetMaxLevel(ch) < IMPLEMENTOR)) {
    cprintf(ch, "Thou art not godly enough.\n\r");
    return;
  }
  if ((adv + GET_LEVEL(victim, lin_class)) > IMPLEMENTOR) {
    cprintf(ch, "Implementor is the highest possible level.\n\r");
    return;
  }
  if (((adv + GET_LEVEL(victim, lin_class)) < 1) &&
      ((adv + GET_LEVEL(victim, lin_class)) != 1)) {
    cprintf(ch, "1 is the lowest possible level.\n\r");
    return;
  }
  cprintf(ch, "You feel generous.\n\r");
  act("$n makes some strange gestures.\n\rA strange feeling comes upon you,"
      "\n\rLike a giant hand, light comes down from\n\rabove, grabbing your "
      "body, that begins\n\rto pulse with coloured lights from inside.\n\rYo"
      "ur head seems to be filled with daemons\n\rfrom another plane as your"
      " body dissolves\n\rinto the elements of time and space itself.\n\rSudde"
      "nly a silent explosion of light snaps\n\ryou back to reality. You fee"
      "l slightly\n\rdifferent.", FALSE, ch, 0, victim, TO_VICT);

  if (GET_LEVEL(victim, lin_class) == 0) {
    do_start(victim);
  } else {
    if (GET_LEVEL(victim, lin_class) < IMPLEMENTOR) {
      int amount_needed, amount_have;

      amount_needed = titles[lin_class][GET_LEVEL(victim, lin_class) + adv].exp + 1;
      amount_have = GET_EXP(victim);
      gain_exp_regardless(victim, amount_needed - amount_have, lin_class);
      cprintf(ch, "Character is now advanced.\n\r");
    } else {
      cprintf(victim, "Some idiot just tried to advance your level.\n\r");
      cprintf(ch, "IMPOSSIBLE! IDIOTIC!\n\r");
    }
  }
}

void do_reroll(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  char buf[100];

  if (IS_NPC(ch))
    return;

  if (IS_IMMORTAL(ch)) {
    only_argument(argument, buf);
    if (!*buf)
      cprintf(ch, "Who do you wish to reroll?\n\r");
    else if (!(victim = get_char(buf)))
      cprintf(ch, "No-one by that name in the world.\n\r");
    else {
      cprintf(ch, "Rerolled...\n\r");
      roll_abilities(victim);
    }
  } else {
    cprintf(ch, "You feel... different!\n\r");
    roll_abilities(ch);
  }
}

void do_restore_all(struct char_data *ch, char *arg, int cmd)
{
  do_restore(ch, "all", 0);
}

void restore_one_victim(struct char_data *victim)
{
  int i;

  GET_MANA(victim) = GET_MAX_MANA(victim);
  if (!affected_by_spell(victim, SPELL_AID)) {
    GET_HIT(victim) = GET_MAX_HIT(victim);
  } else {
    if (GET_HIT(victim) < GET_MAX_HIT(victim))
      GET_HIT(victim) = GET_MAX_HIT(victim);
  }
  GET_MOVE(victim) = GET_MAX_MOVE(victim);
  if (IS_NPC(victim))
    return;

  if (GetMaxLevel(victim) < LOW_IMMORTAL) {
    GET_COND(victim, THIRST) = 24;
    GET_COND(victim, FULL) = 24;
  } else {
    GET_COND(victim, THIRST) = -1;
    GET_COND(victim, FULL) = -1;
  }
  if (GetMaxLevel(victim) >= CREATOR) {
    if (GetMaxLevel(victim) >= LOKI) {
      for (i = 0; i < MAX_SKILLS; i++) {
        victim->skills[i].learned = 100;
        victim->skills[i].recognise = TRUE;
      }
      victim->abilities.str_add = 100;
      victim->abilities.intel = 25;
      victim->abilities.wis = 25;
      victim->abilities.dex = 25;
      victim->abilities.str = 25;
      victim->abilities.con = 25;
      victim->tmpabilities = victim->abilities;
    } else
      for (i = 0; i < MAX_SKILLS; i++) {
        victim->skills[i].learned = number(50,100);
        victim->skills[i].recognise = TRUE;
      }
    if (GetMaxLevel(victim) >= LOKI) {
      if((strcasecmp(GET_NAME(victim), "Quixadhal"))) {
        register int x;
        cprintf(victim, "Fool!  You DARE challenge the Dread Lord?\n\r");
        for(x= 0; x< ABS_MAX_CLASS; x++)
          if(HasClass(victim, 1<<x)) GET_LEVEL(victim, x) = LOW_IMMORTAL;
        save_char(victim, NOWHERE);
      }
    }
  }
  update_pos(victim);
}

void do_restore(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *victim;
  struct descriptor_data *i;
  char buf[256];

  void update_pos(struct char_data *victim);

  only_argument(argument, buf);
  if (!*buf) {
    cprintf(ch, "Who do you wish to restore?\n\r");
  } else if (!strcasecmp(buf, "all")) {
    for (i = descriptor_list; i; i = i->next) {
      if (/* i->character != ch && */ !i->connected &&
          i->character != board_kludge_char) {
        victim = i->character;
        restore_one_victim(victim);
        if(INVIS_LEVEL(victim) < GetMaxLevel(ch))
          cprintf(ch, "%s restored.\n\r", GET_NAME(victim));
        act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
      }
    }
  } else if(GetMaxLevel(ch) < GOD) {
    cprintf(ch, "You have not the power to restore a single mortal!\n\r");
  } else if (!(victim = get_char(buf))) {
    cprintf(ch, "No-one by that name in the world.\n\r");
  } else {
    restore_one_victim(victim);
    if(INVIS_LEVEL(victim) < GetMaxLevel(ch))
      cprintf(ch, "%s restored.\n\r", GET_NAME(victim));
    act("You have been fully healed by $N!", FALSE, victim, 0, ch, TO_CHAR);
  }
}

void do_show_logs(struct char_data *ch, char *argument, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_LOGS)) {
    cprintf(ch, "You will no longer recieve the logs to your screen.\n\r");
    REMOVE_BIT(ch->specials.act, PLR_LOGS);
    return;
  } else {
    cprintf(ch, "You WILL recieve the logs to your screen.\n\r");
    SET_BIT(ch->specials.act, PLR_LOGS);
    return;
  }
}

void do_noshout(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf || IS_MORTAL(ch))
    if (IS_SET(ch->specials.act, PLR_NOSHOUT)) {
      cprintf(ch, "You can now hear shouts again.\n\r");
      REMOVE_BIT(ch->specials.act, PLR_NOSHOUT);
    } else {
      cprintf(ch, "From now on, you won't hear shouts.\n\r");
      SET_BIT(ch->specials.act, PLR_NOSHOUT);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    cprintf(ch, "Couldn't find any such creature.\n\r");
  else if (IS_NPC(vict))
    cprintf(ch, "Can't do that to a beast.\n\r");
  else if (GetMaxLevel(vict) >= GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else if (IS_SET(vict->specials.act, PLR_NOSHOUT) &&
	   (GetMaxLevel(ch) >= SAINT)) {
    cprintf(vict, "You can shout again.\n\r");
    cprintf(ch, "NOSHOUT removed.\n\r");
    REMOVE_BIT(vict->specials.act, PLR_NOSHOUT);
  } else if (GetMaxLevel(ch) >= SAINT) {
    cprintf(vict, "The gods take away your ability to shout!\n\r");
    cprintf(ch, "NOSHOUT set.\n\r");
    SET_BIT(vict->specials.act, PLR_NOSHOUT);
  } else {
    cprintf(ch, "Sorry, you can't do that\n\r");
  }
}

void do_pager(struct char_data *ch, char *arg, int cmd)
{
  if (IS_SET(ch->specials.act, PLR_PAGER)) {
    cprintf(ch, "You stop using the Wiley Pager.\n\r");
    REMOVE_BIT(ch->specials.act, PLR_PAGER);
  } else {
    cprintf(ch, "You now USE the Wiley Pager.\n\r");
    SET_BIT(ch->specials.act, PLR_PAGER);
  }
}

void do_nohassle(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_NOHASSLE)) {
      cprintf(ch, "You can now be hassled again.\n\r");
      REMOVE_BIT(ch->specials.act, PLR_NOHASSLE);
    } else {
      cprintf(ch, "From now on, you won't be hassled.\n\r");
      SET_BIT(ch->specials.act, PLR_NOHASSLE);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    cprintf(ch, "Couldn't find any such creature.\n\r");
  else if (IS_NPC(vict))
    cprintf(ch, "Can't do that to a beast.\n\r");
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    cprintf(ch, "The implementor won't let you set this on mortals...\n\r");
}

void do_stealth(struct char_data *ch, char *argument, int cmd)
{
  struct char_data *vict;
  struct obj_data *dummy;
  char buf[MAX_INPUT_LENGTH];

  if (IS_NPC(ch))
    return;

  only_argument(argument, buf);

  if (!*buf)
    if (IS_SET(ch->specials.act, PLR_STEALTH)) {
      cprintf(ch, "STEALTH mode OFF.\n\r");
      REMOVE_BIT(ch->specials.act, PLR_STEALTH);
    } else {
      cprintf(ch, "STEALTH mode ON.\n\r");
      SET_BIT(ch->specials.act, PLR_STEALTH);
  } else if (!generic_find(argument, FIND_CHAR_WORLD, ch, &vict, &dummy))
    cprintf(ch, "Couldn't find any such creature.\n\r");
  else if (IS_NPC(vict))
    cprintf(ch, "Can't do that to a beast.\n\r");
  else if (GetMaxLevel(vict) > GetMaxLevel(ch))
    act("$E might object to that.. better not.", 0, ch, 0, vict, TO_CHAR);
  else
    cprintf(ch, "The implementor won't let you set this on mortals...\n\r");

}

static void print_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  char buf[MAX_STRING_LENGTH];
  int dink, bits, scan;

  sprintf(buf, "%5d %4d %-12s %s", rp->number, rnum,
	  sector_types[rp->sector_type], rp->name);
  strcat(buf, " [");

  dink = 0;
  for (bits = rp->room_flags, scan = 0; bits; scan++) {
    if (bits & (1 << scan)) {
      if (dink)
	strcat(buf, " ");
      strcat(buf, room_bits[scan]);
      dink = 1;
      bits ^= (1 << scan);
    }
  }
  strcat(buf, "]\n\r");

  append_to_string_block(sb, buf);
}

static void print_death_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags & DEATH)
    print_room(rnum, rp, sb);
}

static void print_private_room(int rnum, struct room_data *rp, struct string_block *sb)
{
  if (rp && rp->room_flags & PRIVATE)
    print_room(rnum, rp, sb);
}

static void show_room_zone(int rnum, struct room_data *rp,
			   struct show_room_zone_struct *srzs)
{
  char buf[MAX_STRING_LENGTH];

  if (!rp || rp->number < srzs->bottom || rp->number > srzs->top)
    return;			       /* optimize later */

  if (srzs->blank && (srzs->lastblank + 1 != rp->number)) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  if (1 == sscanf(rp->name, "%d", &srzs->lastblank) &&
      srzs->lastblank == rp->number) {
    if (!srzs->blank) {
      srzs->startblank = srzs->lastblank;
      srzs->blank = 1;
    }
    return;
  } else if (srzs->blank) {
    sprintf(buf, "rooms %d-%d are blank\n\r", srzs->startblank, srzs->lastblank);
    append_to_string_block(srzs->sb, buf);
    srzs->blank = 0;
  }
  print_room(rnum, rp, srzs->sb);
}

void do_show(struct char_data *ch, char *argument, int cmd)
{
  int zone;
  char buf[MAX_STRING_LENGTH], zonenum[MAX_INPUT_LENGTH];
  struct index_data *which_i;
  int bottom = 0, top = 0, topi;
  struct string_block sb;

  if (IS_NPC(ch))
    return;

  argument = one_argument(argument, buf);
  init_string_block(&sb);

  if (is_abbrev(buf, "zones")) {
    struct zone_data *zd;
    int bottom = 0;

    append_to_string_block(&sb,
			   "# Zone   name                                lifespan age     rooms     reset\n\r");

    for (zone = 0; zone <= top_of_zone_table; zone++) {
      char *mode;

      zd = zone_table + zone;
      switch (zd->reset_mode) {
      case 0:
	mode = "never";
	break;
      case 1:
	mode = "ifempty";
	break;
      case 2:
	mode = "always";
	break;
      default:
	mode = "!unknown!";
	break;
      }
      sprintf(buf, "%4d %-40s %4dm %4dm %6d-%-6d %s\n\r", zone, zd->name,
	      zd->lifespan, zd->age, bottom, zd->top, mode);
      append_to_string_block(&sb, buf);
      bottom = zd->top + 1;
    }
  } else if ((is_abbrev(buf, "objects") &&
	     (which_i = obj_index, topi = top_of_objt)) ||
	     (is_abbrev(buf, "mobiles") &&
	     (which_i = mob_index, topi = top_of_mobt)) ) {
    int objn;
    struct index_data *oi;

    only_argument(argument, zonenum);
    zone = -1;
    if (1 == sscanf(zonenum, "%i", &zone) && (zone < 0 || zone > top_of_zone_table)) {
      append_to_string_block(&sb, "That is not a valid zone_number\n\r");
      return;
    }
    if (zone >= 0) {
      bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      top = zone_table[zone].top;
    }
    append_to_string_block(&sb, "VNUM  rnum count names\n\r");
    for (objn = 0; objn <= topi; objn++) {
      oi = which_i + objn;
      if ((zone >= 0 && (oi->virtual < bottom || oi->virtual > top)) ||
	  (zone < 0 && !isname(zonenum, oi->name)))
	continue;		       /* optimize later */
      sprintf(buf, "%5d %4d %3d  %s\n\r", oi->virtual, objn, oi->number, oi->name);
      append_to_string_block(&sb, buf);
    }
  } else if (is_abbrev(buf, "rooms")) {
    only_argument(argument, zonenum);
    append_to_string_block(&sb, "VNUM  rnum type         name [BITS]\n\r");
    if (is_abbrev(zonenum, "death")) {
      hash_iterate(&room_db, (funcp)print_death_room, &sb);
    } else if (is_abbrev(zonenum, "private")) {
      hash_iterate(&room_db, (funcp)print_private_room, &sb);
    } else if (1 != sscanf(zonenum, "%i", &zone) || zone < 0 || zone > top_of_zone_table) {
      append_to_string_block(&sb, "I need a zone number with this command\n\r");
    } else {
      struct show_room_zone_struct srzs;

      srzs.bottom = zone ? (zone_table[zone - 1].top + 1) : 0;
      srzs.top = zone_table[zone].top;
      srzs.blank = 0;
      srzs.sb = &sb;
      hash_iterate(&room_db, (funcp)show_room_zone, &srzs);
      if (srzs.blank) {
	sprintf(buf, "rooms %d-%d are blank\n\r", srzs.startblank, srzs.lastblank);
	append_to_string_block(&sb, buf);
	srzs.blank = 0;
      }
    }
  } else {
    append_to_string_block(&sb, "Usage:\n\r"
			   "  show zones\n\r"
			 "  show (objects|mobiles) (zone#|name)\n\r"
			   "  show rooms (zone#|death|private)\n\r");
  }
  page_string_block(&sb, ch);
  destroy_string_block(&sb);
}

void do_debug(struct char_data *ch, char *argument, int cmd)
{
  char arg[MAX_INPUT_LENGTH];
  int i;

  i = 0;
  one_argument(argument, arg);
  i = atoi(arg);

  if (i < 0 || i > 2) {
    cprintf(ch, "valid values are 0, 1 and 2\n\r");
  } else {
    if (i == 1) {
      DEBUG = 1;
      DEBUG2 = 0;
    } else if (i == 2) {
      DEBUG = 0;
      DEBUG2 = 1;
    } else {
      DEBUG = 0;
      DEBUG2 = 0;
    }
    cprintf(ch, "debug level set to %d\n\r", i);
  }
}

void do_invis(struct char_data *ch, char *argument, int cmd)
{
  int level;

  if (scan_number(argument, &level)) {
    if (level <= 0)
      level = 0;
    else {
      if (level >= GetMaxLevel(ch)) {
	cprintf(ch, "Sorry, you cant invis that high yet!\n\r");
	return;
      }
    }
    ch->invis_level = level;
    cprintf(ch, "Invis level set to %d.\n\r", level);
  } else {
    if (ch->invis_level > 0) {
      ch->invis_level = 0;
      cprintf(ch, "You are now totally VISIBLE.\n\r");
    } else {
      ch->invis_level = GetMaxLevel(ch) - 1;
      cprintf(ch, "You are now invisible to level %d.\n\r", GetMaxLevel(ch) - 1);
    }
  }
}

void do_reset(struct char_data *ch, char *argument, int cmd)
{
  int start, finish, i;
  struct room_data *rp;
  char start_level[256], finish_level[256];

  if(DEBUG)
    log("do_reset");
  if (IS_NPC(ch))
    return;
  argument = one_argument(argument, start_level);
  if(!strcasecmp(start_level, "all")) {
    start= 0;
    finish= top_of_zone_table;
  } else if(*start_level) {
    start= atoi(start_level);
    if(start < 0)
      start= 0;
    if(start > top_of_zone_table) 
      start= top_of_zone_table;
    argument = one_argument(argument, finish_level);
    if(*finish_level) {
      finish= atoi(finish_level);
      if(finish < start)
        finish= start;
      if(finish > top_of_zone_table)
        finish= top_of_zone_table;
    } else {
      finish= start;
    }
  } else {
    if((rp= real_roomp(ch->in_room))) {
      start= finish= rp->zone;
    } else {
      return;
    }
  }
  for (i= start; i <= finish; i++) {
    if(zone_table[i].reset_mode) {
      reset_zone(i);
    }
  }
  if(start != finish) {
    cprintf(ch, "You have reset Zones %d through %d.\n\r", start, finish);
    log("Reset of Zones [#%d] to [#%d] by %s.", start, finish, GET_NAME(ch));
  } else {
    cprintf(ch, "You have reset Zone %d.\n\r", start);
    log("Reset of Zone [#%d] by %s.", start, GET_NAME(ch));
  }
}

static void zone_purge_effect( int rnum, struct room_data *rp, int *zones )
{
  struct char_data *vict, *next_v;
  struct obj_data *obj, *next_o;

  if(!rp || rp->zone < zones[0] || rp->zone > zones[1])
    return;
  rprintf(rnum, "Flames shoot skyward all around you, and it grows quiet.\n\r");

  for (vict = rp->people; vict; vict = next_v) {
    next_v = vict->next_in_room;
    if (IS_NPC(vict) && (!IS_SET(vict->specials.act, ACT_POLYSELF)))
      extract_char(vict);
  }
  for (obj = rp->contents; obj; obj = next_o) {
    next_o = obj->next_content;
    extract_obj(obj);
  }
}

void do_zone_purge(struct char_data *ch, char *argument, int cmd)
{
  int zones[2];
  struct room_data *rp;
  char start_level[256], finish_level[256];

  if(DEBUG)
    log("do_zone_purge");
  if (IS_NPC(ch))
    return;
  argument = one_argument(argument, start_level);
  if(!strcasecmp(start_level, "all")) {
    zones[0]= 0;
    zones[1]= top_of_zone_table;
  } else if(*start_level) {
    zones[0]= atoi(start_level);
    if(zones[0] < 0)
      zones[0]= 0;
    if(zones[0] > top_of_zone_table) 
      zones[0]= top_of_zone_table;
    argument = one_argument(argument, finish_level);
    if(*finish_level) {
      zones[1]= atoi(finish_level);
      if(zones[1] < zones[0])
        zones[1]= zones[0];
      if(zones[1] > top_of_zone_table)
        zones[1]= top_of_zone_table;
    } else {
      zones[1]= zones[0];
    }
  } else {
    if((rp= real_roomp(ch->in_room))) {
      zones[0]= zones[1]= rp->zone;
    } else {
      return;
    }
  }
  hash_iterate(&room_db, (funcp)zone_purge_effect, zones);
  if(zones[0] != zones[1]) {
    cprintf(ch, "You have cleaned Zones %d through %d.\n\r", zones[0], zones[1]);
    log("Purge of Zones [#%d] to [#%d] by %s.", zones[0], zones[1], GET_NAME(ch));
  } else {
    cprintf(ch, "You have cleaned Zone %d.\n\r", zones[0]);
    log("Purge of Zone [#%d] by %s.", zones[0], GET_NAME(ch));
  }
}

void do_not_yet_implemented(struct char_data *ch, char *argument, int cmd)
{
  cprintf(ch, "This command is not yet implemented.\n\r");
}

void do_setreboot(struct char_data *ch, char *argument, int cmd)
{
  FILE *pfd;
  int first, second;
  char first_str[256], second_str[256];

  if(DEBUG)
    log("do_reset");
  if (IS_NPC(ch))
    return;
  argument = one_argument(argument, first_str);
  if(*first_str) {
    first= atoi(first_str);
    if(first < 0)
      first= 0;
    if(first > 23) 
      first= 23;
    argument = one_argument(argument, second_str);
    if(*second_str) {
      second= atoi(second_str);
      if(second < 0)
        second= 0;
      if(second > 23)
        second= 23;
    } else {
      second= first;
    }
  } else {
    first= 7;
    second= 19;
  }
  REBOOT_AT1= first;
  REBOOT_AT2= second;
  if(!(pfd= fopen(REBOOTTIME_FILE, "w"))) {
    log("Cannot save reboot times!");
  } else {
    fprintf(pfd, "%d %d\n", REBOOT_AT1, REBOOT_AT2);
    fclose(pfd);
  }
  cprintf(ch, "You have set reboot times of %02d:00 and %02d:00.\n\r",
          REBOOT_AT1, REBOOT_AT2);
  log("Reboot times %02d:00 and %02d:00 set by %s.",
          REBOOT_AT1, REBOOT_AT2, GET_NAME(ch));
}

