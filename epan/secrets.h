/* secrets.h
 * Secrets management and processing.
 * Copyright 2018, Peter Wu <peter@lekensteyn.nl>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1998 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __SECRETS_H__
#define __SECRETS_H__

#include <glib.h>
#include "ws_symbol_export.h"

/**
 * Interfaces for management and processing of secrets provided by external
 * sources (wiretap, key files, HSMs, etc.). Dissectors can register themselves
 * as consumers of these secrets.
 *
 * Future idea: provide helper functions to manage external files. Typically
 * these secrets can be erased when the file is truncated or deleted+created.
 * Additionally, these secrets are not tied to the lifetime of a capture file.
 *
 * Future idea: add a method for dissectors to mark secrets as "in use" such
 * that unused entries can be removed when saving those secrets to file.
 * Intended use case: read large TLS key log file (which is infrequently
 * truncated by the user) and store only the bare minimum keys.
 */

void secrets_init(void);
void secrets_cleanup(void);

#if 0
/**
 * Lifetime of provided secrets.
 * HSM: tie information to epan scope? (but if disconnected, clear state?)
 * wiretap pcang DSB: scoped to (capture) file.
 * tls.keylog_file pref: epan-scoped (but if the file is deleted, clear it)
 */
enum secrets_scope {
    SECRETS_SCOPE_EPAN,
    SECRETS_SCOPE_FILE,
};
#endif

#ifdef HAVE_LIBGNUTLS
/** Identifier for a RSA public key (a SHA-1 hash). */
struct cert_key_id {
    guint8 key_id[20];
};
typedef struct cert_key_id cert_key_id_t;
#endif  /* HAVE_LIBGNUTLS */


/**
 * Callback for the wiretap secrets provider (wtap_new_secrets_callback_t).
 */
WS_DLL_PUBLIC void
secrets_wtap_callback(guint32 secrets_type, const void *secrets, guint size);

/**
 * Receives a new block of secrets from an external source (wiretap or files).
 */
typedef void (*secrets_block_callback_t)(const void *secrets, guint size);

/**
 * Registers a consumer for pcapng Decryption Secrets Block (DSB). Only one
 * dissector can register a type.
 *
 * @param secrets_type A Secrets Type as defined in wiretap/secrets-types.h
 * @param cb Callback to be invoked for new secrets.
 */
void secrets_register_type(guint32 secrets_type, secrets_block_callback_t cb);

#ifdef HAVE_LIBGNUTLS
/** Returns a new hash table, mapping cert_key_id_t -> gnutls_privkey_t. */
GHashTable *privkey_hash_table_new(void);

/**
 * Tries to decrypt the given buffer using a private key identified by key_id.
 * The private key was loaded through the 'rsa_keys' UAT.
 *
 * @param key_id Identifier for the public key.
 * @param encr Encrypted input.
 * @param encr_len Size of encrypted input.
 * @param out Decrypted contents on success, free with g_free.
 * @param out_len Size of decrypted contents on success.
 * @return 0 if a private key was available and decryption succeeded, a negative
 * error code otherwise.
 */
WS_DLL_PUBLIC int
secrets_rsa_decrypt(const cert_key_id_t *key_id, const guint8 *encr, int encr_len, guint8 **out, int *out_len);
#endif  /* HAVE_LIBGNUTLS */

#endif /* __SECRETS_H__ */
