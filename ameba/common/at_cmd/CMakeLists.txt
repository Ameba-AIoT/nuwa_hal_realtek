# Copyright (c) 2026 Realtek Semiconductor Corp.
# SPDX-License-Identifier: Apache-2.0

zephyr_library_named(ameba_at_cmd)
zephyr_library_sources(atcmd_parser.c)
zephyr_library_sources_ifdef(CONFIG_ATCMD_SECURE atcmd_secure.c)
