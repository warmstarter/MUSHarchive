/* convflags.c */

/* old flags */

/* Exit flags */
#define OLD_EXIT_KEY 0x8

/* Player flags */
#define OLD_PLAYER_BUILD 0x8
#define OLD_PLAYER_MYOPIC 0x20000
#define OLD_PLAYER_NOSPOOF 0x200000
#define OLD_PLAYER_TERSE 0x2000000
#define OLD_PLAYER_SUSPECT 0x4000000

/* Object flags */
#define OLD_THING_KEY  0x8
#define OLD_THING_DEST_OK 0x200
#define OLD_THING_IMMORTAL 0x2000
#define OLD_THING_PUPPET   0x20000
#define OLD_THING_LISTEN 0x200000

/* Room flags */
#define OLD_ROOM_FLOATING 0x8
#define OLD_ROOM_ABODE 0x200
#define OLD_ROOM_JUMP_OK 0x2000
#define OLD_ROOM_TRANSPARENT 0x20000
#define OLD_ROOM_UNFIND 0x2000000
#define OLD_ROOM_NO_TEL 0x4000000

static void
conv_newflags(current, nflags, ntoggles, npowers)
    int current;
    int *nflags;
    int *ntoggles;
    int *npowers;
{
  int flags, toggles, powers;
  flags = current;
  toggles = powers = 0;

  switch (flags & TYPE_MASK) {
  case TYPE_EXIT:
    if (current & OLD_EXIT_KEY)
      flags &= ~OLD_EXIT_KEY;
    break;
  case TYPE_PLAYER:
    if (current & OLD_PLAYER_BUILD) {
      flags &= ~OLD_PLAYER_BUILD;
      powers |= CAN_BUILD;
    }
    if (current & OLD_PLAYER_MYOPIC) {
      flags &= ~OLD_PLAYER_MYOPIC;
      toggles |= PLAYER_MYOPIC;
    }
    if (current & OLD_PLAYER_NOSPOOF) {
      flags &= ~OLD_PLAYER_NOSPOOF;
      toggles |= PLAYER_NOSPOOF;
    }
    if (current & OLD_PLAYER_TERSE) {
      flags &= ~OLD_PLAYER_TERSE;
      toggles |= PLAYER_TERSE;
    }
    if (current & OLD_PLAYER_SUSPECT) {
      flags &= ~OLD_PLAYER_SUSPECT;
      toggles |= PLAYER_SUSPECT;
    }
    if (current & PLAYER_GAGGED) {
      flags &= ~PLAYER_GAGGED;
      toggles |= PLAYER_GAGGED;
    }
    if (current & PLAYER_MONITOR) {
      flags &= ~PLAYER_MONITOR;
      toggles |= PLAYER_MONITOR;
    }
    if (current & AUDIBLE) {
      flags &= ~AUDIBLE;
      toggles |= PLAYER_ANSI;
    }
    break;
  case TYPE_THING:
    if (current & OLD_THING_KEY)
      flags &= ~OLD_THING_KEY;
    if (current & OLD_THING_DEST_OK) {
      flags &= ~OLD_THING_DEST_OK;
      toggles |= THING_DEST_OK;
    }
    if (current & OLD_THING_IMMORTAL) {
      flags &= ~OLD_THING_IMMORTAL;
      powers |= IMMORTAL;
    }
    if (current & OLD_THING_PUPPET) {
      flags &= ~OLD_THING_PUPPET;
      toggles |= THING_PUPPET;
    }
    if (current & OLD_THING_LISTEN) {
      flags &= ~OLD_THING_LISTEN;
      toggles |= THING_LISTEN;
    }
    break;
  case TYPE_ROOM:
    if (current & OLD_ROOM_FLOATING) {
      flags &= ~OLD_ROOM_FLOATING;
      toggles |= ROOM_FLOATING;
    }
    if (current & OLD_ROOM_ABODE) {
      flags &= ~OLD_ROOM_ABODE;
      toggles |= ROOM_ABODE;
    }
    if (current & OLD_ROOM_JUMP_OK) {
      flags &= ~OLD_ROOM_JUMP_OK;
      toggles |= ROOM_JUMP_OK;
    }
    if (current & OLD_ROOM_TRANSPARENT) {
      flags &= ~OLD_ROOM_TRANSPARENT;
      flags |= TRANSPARENTED;
    }
    if (current & OLD_ROOM_UNFIND) {
      flags &= ~OLD_ROOM_UNFIND;
      flags |= UNFIND;
    }
    if (current & OLD_ROOM_NO_TEL) {
      flags &= ~OLD_ROOM_NO_TEL;
      toggles |= ROOM_NO_TEL;
    }
    if (current & ROOM_TEMPLE) {
      flags &= ~ROOM_TEMPLE;
      /* TEMPLE is no longer in hardcode */
    }
    break;
  }

  *nflags = flags;
  *ntoggles = toggles;
  *npowers = powers;
}
