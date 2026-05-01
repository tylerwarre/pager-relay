#include <string.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include "email.h"

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

int email_send(EmailSettings *s, char *body, EmailType type) {
    CURL *curl = NULL;
    CURLcode ret = CURLE_OK;
    struct curl_slist *recipients = NULL;
    EmailCtx upload_ctx = { 0 };

    // TODO: Testing Start
    char username[255];
    char password[255];
    char date[36];
    printf("Username: ");
    fgets(username, 255, stdin);
    printf("Password: ");
    fgets(password, 255, stdin);
    password[strlen(password)-1] = '\0';
    username[strlen(username)-1] = '\0';

    char *msg = calloc(2000, 1);

    time_t t;
    struct tm *tmp;
    t = time(NULL);
    tmp = localtime(&t);
    strftime(date, sizeof(date), "%a, %d %b %Y %T %z", tmp);

    sprintf(msg, EMAIL_FMT, "tyler@warrens.one", username, date, "Brightwheel Relay", body);
    //msg = realloc(msg, strlen(msg+1));
    msg[367] = '\0';

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

        // TODO get from settings
        // Specify credentials
        curl_easy_setopt(curl, CURLOPT_USERNAME, username);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, password);

        // TODO get from settings
        // Specify to/from
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, username);
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
