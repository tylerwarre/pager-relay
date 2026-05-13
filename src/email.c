#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "include/email.h"
#include "include/util.h"
#include "include/error.h"

static size_t cb_read(char *ptr, size_t size, size_t nmemb, void *userp) {
    EmailCtx *ctx = (EmailCtx *)userp;
    const char *data = NULL;
    size_t room = size * nmemb;
    size_t len = 0;

    if ((size == 0) || (nmemb == 0) || (room < 1)) {
        return 0;
    }

    // Set data to the current position in upload
    data = ctx->msg + ctx->bytes_read;

    len = strlen(data);
    // If the internal buffer can't fit all the data,
    //  send the max data to fill the internal buffer?
    if (room < len) {
        len = room;
    }

    memcpy(ptr, data, len);
    ctx->bytes_read += len;

    return len;
}

static int email_len(EmailSettings *s, char *body, EmailType type) {
    int len = 0;
    List *ptr = NULL;

    // Get length of FMT string w/o placeholders
    // -10 is used because there are five 2 character placeholders
    len += strlen(EMAIL_FMT) - 10;

    // Get length of To
    len += util_list_len(s->receipients, ", ");

    // Get length of From
    len += strlen(s->sender);

    // Get length of Date
    // date is a fixed format (e.g. "Fri, 21 Nov 1997 09:55:06 -0600")
    len += date_len;

    // Get length of Subject
    if (type == brightwheel) {
        // get the length of the canonical sender name
        len += strlen(SUBJ_BRIGHTWHEEL);
    }

    // Get length of Body
    len += strlen(body);

    return len;
}

static char* prepare_email(EmailSettings *s, char *body, EmailType type) {
    bool err = false;
    int len = 0;
    time_t t = 0;
    struct tm *tmp = {0};
    char date[date_len+1];
    char *to = NULL;
    const char *subj = NULL;
    char *msg = NULL;


    while (true) {
        // Get the Date string
        t = time(NULL);
        tmp = localtime(&t);
        if (strftime(date, sizeof(date), "%a, %d %b %Y %T %z", tmp) == 0) {
            fprintf(stderr, "[%s] Unable to build date string\n", __func__);
            err = true;
            break;
        }

        // Get To string
        if ((to = util_list_tostring(s->receipients, ", ")) == NULL) {
            err = true;
            break;
        }

        // Get the total length of the email message
        len = email_len(s, body, type);

        // Get Subject string
        if (type == brightwheel) {
            subj = SUBJ_BRIGHTWHEEL;
        }
        else {
            err = true;
            break;
        }

        // Allocate memory for msg string
        if ((msg = calloc(len+1, sizeof(char))) == NULL) {
            fprintf(stderr, "[%s] Unable to allocate memory for email message\n", __func__);
            err = true;
            break;
        }

        sprintf(msg, EMAIL_FMT, to, s->sender, date, subj, body);
        break;
    }

    if (to != NULL) {
        free(to);
        to = NULL;
    }

    if (err) {
        if (msg != NULL) {
            free(msg);
            msg = NULL;
        }
    }

    return msg;
}

int email_send(EmailSettings *s, char *body, EmailType type) {
    char *msg = NULL;
    List *ptr = NULL;
    CURL *curl = NULL;
    CURLcode ret = CURLE_OK;
    struct curl_slist *recipients = NULL;
    EmailCtx upload_ctx = { 0 };

    if ((msg = prepare_email(s, body, type)) == NULL) {
        return E_EMAIL;
    }

    upload_ctx.msg = msg;
    // TODO: Testing End

    while (true) {
        if ((curl = curl_easy_init()) == NULL) {
            fprintf(stderr, "[%s] Unable to initalize curl for request\n", __func__);
            ret = (int)CURLE_FAILED_INIT;
            break;
        }

        // Configured for implicit TLS
        curl_easy_setopt(curl, CURLOPT_USE_SSL, 1L);
        curl_easy_setopt(curl, CURLOPT_URL, "smtps://mail.warrens.one");
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

        // Specify credentials
        curl_easy_setopt(curl, CURLOPT_USERNAME, s->sender);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, s->password);

        // Specify to/from
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, s->sender);

        // TDOO: Conflict between email message email format at SMTP transaction. Probably need to seperate
        //  cononical name and email address in settings
        // build curl list of receipients
        /*
        ptr = s->receipients;
        while (ptr->next != NULL) {
            recipients = curl_slist_append(recipients, ptr->str);
            ptr = ptr->next;
        }
        // Make sure we append the last item in the list
        recipients = curl_slist_append(recipients, ptr->str);
        */
        recipients = curl_slist_append(recipients, "tyler@warrens.one");
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);

        // Configure how the message is uploaded
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, cb_read);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);

        if ((ret = curl_easy_perform(curl)) != CURLE_OK) {
            fprintf(stderr, "[%s] Unable to process curl request: %s\n", __func__, curl_easy_strerror(ret));
            break;
        }
        break;
    }

    if (msg != NULL) {
        free(msg);
        msg = NULL;
    }
    curl_slist_free_all(recipients);
    curl_easy_cleanup(curl);

    return (int)ret;
}
