#ifndef EMAIL_H
#define EMAIL_H

#include "email.h"
#include "settings.h"

#include <curl/curl.h>

// Spec says the max is 1000, but unclear if that includes headers (probably not?)
#define EMAIL_MAX_LEN 500

typedef struct EmailCtx {
    size_t bytes_read;
    char *msg;
} EmailCtx;

typedef enum {
   brightwheel
} EmailType;

// TODO: Update "To/From" to not use <>
static const char *EMAIL_FMT =
    "To: <%s>\r\n"
    "From: <%s>\r\n"
    "Content-Type: text/plain; charset=us-ascii\r\n"
    "Content-Transfer-Encoding: 7bit\r\n"
    "MIME-Version: 1.0\r\n"
    "Date: %s\r\n"
    "Subject: %s\r\n"
    "\r\n"
    "%s"
    "\r\n";

int email_send(EmailSettings *s, char *msg, EmailType type);
static size_t cb_read(char *ptr, size_t size, size_t nmemb, void *userp);

#endif
