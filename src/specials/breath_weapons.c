#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <signal.h>
#include <ctype.h>

#include "include/global.h"
#include "include/bug.h"
#include "include/utils.h"

#include "include/act_off.h"
#include "include/comm.h"
#include "include/db.h"
#include "include/limits.h"
#include "include/multiclass.h"
#include "include/spells.h"

#define _BREATH_WEAPONS_C
#include "include/breath_weapons.h"

static funcp breaths[] = {
  (funcp)cast_acid_breath, NULL,
  (funcp)cast_frost_breath, NULL,
  (funcp)cast_lightning_breath, NULL,
  (funcp)cast_fire_breath, NULL,
  (funcp)cast_acid_breath,
  (funcp)cast_fire_breath,
  (funcp)cast_lightning_breath,
  NULL
};

struct breather breath_monsters[] = {
  {5030, 15, breaths + 6},
  {-1},
};

struct breath_victim *choose_victims(struct char_data *ch,
				     struct char_data *first_victim) {
  /* this is goofy, dopey extraordinaire */
  struct char_data *cons;
  struct breath_victim *head = NULL, *temp = NULL;

  for (cons = real_roomp(ch->in_room)->people; cons; cons = cons->next_in_room) {
    if(!(temp = (void *)malloc(sizeof(*temp)))) {
      log("Malloc failure in special proc breath_victim.  Exiting.");
      kill(getpid(),12);
    }
    temp->ch = cons;
    temp->next = head;
    head = temp;
    if (first_victim == cons) {
      temp->yesno = 1;
    } else if (ch == cons) {
      temp->yesno = 0;
    } else if ((in_group(first_victim, cons) ||
		cons == first_victim->master ||
		cons->master == first_victim) &&
	       (temp->yesno = (3 != dice(1, 5)))) {
      /* group members will get hit 4/5 times */
    } else if (cons->specials.fighting == ch) {
      /* people fighting the dragon get hit 4/5 times */
      temp->yesno = (3 != dice(1, 5));
    } else			       /* bystanders get his 2/5 times */
      temp->yesno = (dice(1, 5) < 3);
  }
  return head;
}

void free_victims(struct breath_victim *head) {
  struct breath_victim *temp;

  while (head) {
    temp = head->next;
    free(head);
    head = temp;
  }
}

int breath_weapon(struct char_data *ch, struct char_data *target,
		  int mana_cost, funcp func) {
  struct breath_victim *hitlist, *scan;
  int victim;

  hitlist = choose_victims(ch, target);

  act("$n rears back and inhales", 1, ch, 0, ch->specials.fighting, TO_ROOM);
  victim = 0;
  for (scan = hitlist; scan; scan = scan->next) {
    if (!scan->yesno ||
	IS_IMMORTAL(scan->ch) ||
	scan->ch->in_room != ch->in_room	/* this should not happen */
      )
      continue;
    victim = 1;
    cast_fear(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
  }

  if (func != NULL && victim) {
    act("$n Breathes...", 1, ch, 0, ch->specials.fighting, TO_ROOM);

    for (scan = hitlist; scan; scan = scan->next) {
      if (!scan->yesno ||
	  IS_IMMORTAL(scan->ch) ||
	  scan->ch->in_room != ch->in_room	/* this could happen if
						 * someone fled, I guess */
	)
	continue;
      func(GetMaxLevel(ch), ch, "", SPELL_TYPE_SPELL, scan->ch, 0);
    }
    GET_MANA(ch) -= mana_cost;
  } else {
    act("$n Breathes...coughs and sputters...",
	1, ch, 0, ch->specials.fighting, TO_ROOM);
    do_flee(ch, "", 0);
  }
  free_victims(hitlist);
  return TRUE;
}

void use_breath_weapon(struct char_data *ch, struct char_data *target,
		      int cost, funcp func) {
  if (GET_MANA(ch) >= 0) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 2) &&
	     (GET_MANA(ch) >= -cost)) {
    breath_weapon(ch, target, cost, func);
  } else if ((GET_HIT(ch) < GET_MAX_HIT(ch) / 4) &&
	     (GET_MANA(ch) >= -2 * cost)) {
    breath_weapon(ch, target, cost, func);
  } else if (GET_MANA(ch) <= -3 * cost) {
    breath_weapon(ch, target, 0, NULL);		/* sputter */
  }
}

int BreathWeapon(struct char_data *ch, int cmd, char *arg) {
  char buf[MAX_STRING_LENGTH];
  struct breather *scan;
  int count;

  if (cmd)
    return FALSE;

  if (ch->specials.fighting &&
      (ch->specials.fighting->in_room == ch->in_room)) {

    for (scan = breath_monsters;
	 scan->vnum >= 0 && scan->vnum != mob_index[ch->nr].virtual;
	 scan++);

    if (scan->vnum < 0) {
      sprintf(buf, "monster %s tries to breath, but isn't listed.",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }
    for (count = 0; scan->breaths[count]; count++);

    if (count < 1) {
      sprintf(buf, "monster %s has no breath weapons",
	      ch->player.short_descr);
      log(buf);
      return FALSE;
    }
    use_breath_weapon(ch, ch->specials.fighting, scan->cost,
		      scan->breaths[dice(1, count) - 1]);
  }
  return TRUE;
}
