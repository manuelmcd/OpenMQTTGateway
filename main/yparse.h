#if !defined(__YPARSE_H__)
#define __YPARSE_H__

#include "jsmn.h"

const int yparser_max_tokens = 128;

typedef struct {
    jsmn_parser jp;
    const char *data;
    jsmntok_t t[yparser_max_tokens];
    int token_count;
    int function_topic_root_token;
    int function_topic_root_len;
    int function_topic_next_attr_token;
    int function_topic_next_attr_idx;
    int node_topic_root_token;
    int node_topic_next_attr_token;
    int node_topic_next_attr_idx;
} yparser;


int yparse(yparser *yp, const char *data);
int yparse_get_function_msg(yparser *yp, char *topicbuf, int tb_max_len, char *valuebuf, int vb_max_len);
int yparse_get_node_msg(yparser *yp, char *topicbuf, int tb_max_len, char *valuebuf, int vb_max_len);

#endif /* !defined(__YPARSE_H__) */
