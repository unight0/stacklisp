#!/bin/env python3

ATYP_ROOT, ATYP_BLOCK, ATYP_APPL, ATYP_LIST, ATYP_FUN, ATYP_ATOM, ATYP_IF = range(7)


ASTTYPE = {
    ATYP_ROOT: 'Root',
    ATYP_BLOCK: 'Block',
    ATYP_APPL: 'Function application',
    ATYP_LIST: 'List',
    ATYP_FUN: 'Function',
    ATYP_IF: 'If',
    ATYP_ATOM: 'Atom'
}
