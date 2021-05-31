/*! \file slave.h
 * \brief Shared definitions between the main server and the dns / ident slave.
 *
 * $Id: slave.h 654 2007-01-05 04:18:02Z brazilofmux $
 *
 * This enum doesn't actually appear to be used for anything.
 */

enum {
    SLAVE_IDENTQ = 'i',
    SLAVE_IPTONAME = 'h'
};
