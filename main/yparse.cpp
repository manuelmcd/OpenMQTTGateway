#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "ArduinoLog.h"

#define JSMN_STATIC
#include "jsmn.h"
#include "yparse.h"

#define ETOOLONG -1

int tokcmp(const char *js, jsmntok_t *tok, const char *s2)
{
   if(strlen(s2) > (tok->end - tok->start)) return -1;
   return strncmp(js + tok->start, s2, tok->end - tok->start);
}

/* Convert unrecognised primitives to strings, ala yaml */
static void maybe_convert_primitive_to_string(const char *js, jsmntok_t *token)
{
    if(
       tokcmp(js, token, "null") == 0 ||
       tokcmp(js, token, "true") == 0 ||
       tokcmp(js, token, "false") == 0
    ) {
//	printf("'%.*s' is one of null,true,false\n", token->end-token->start, js+token->start);
	return;
    }
    int is_num = 1;
    int i=token->start;
    if(js[i] == '-' && (token->end - token->start) > 1) {
	i++;
    }
    for(;i < token->end; i++) {
	if(!(js[i] >= '0' && js[i] <= '9')) {
	    is_num = 0;
	    break;
	}
    }
    if(is_num != 0) {
//	printf("'%.*s' is a number\n", token->end-token->start, js+token->start);
	return;
    }
    // Assume it's a string
//    printf("Converting '%.*s' to string\n", token->end-token->start, js+token->start);
    token->type = JSMN_STRING;
    return;
}

static int dumpjson(char *buf, int maxlen, const char *js, jsmntok_t *t, size_t count) {
  int i, j, k;
  jsmntok_t *key;
  char *bufend = buf + maxlen;
  if (count == 0) {
    return 0;
  }
  int len = t->end - t->start;
  if (t->type == JSMN_PRIMITIVE) {
      maybe_convert_primitive_to_string(js, t);
  }
  if (t->type == JSMN_PRIMITIVE) {
      if(len >= maxlen) {
	  return ETOOLONG;
      }
      strncpy(buf, js + t->start, len);
      buf[len] = 0;
      return 1;
  } else if (t->type == JSMN_STRING) {
      if(len+2 >= maxlen) {
	  return ETOOLONG;
      }
      strncpy(buf+1, js + t->start, len);
      buf[0] = buf[len+1] = '"';
      buf[len+2] = 0;
      return 1;
  } else if (t->type == JSMN_OBJECT) {
      // Min possible length is 2.
      if(maxlen < 3) {
	  return ETOOLONG;
      }
      buf[0] = '{';
      buf++;
      j = 0;
      for (i = 0; i < t->size; i++) {
          key = t + 1 + j;
	  int r = dumpjson(buf, bufend-buf, js, key, count - j);
	  if(r == ETOOLONG) {
	      return r;
	  }
	  j += r;
	  buf += strlen(buf);
	  buf[0] = ':';
	  buf[1] = ' ';
	  buf += 2;
	  if (key->size > 0) {
	      int r = dumpjson(buf, bufend-buf, js, t + 1 + j, count - j);
	      if(r == ETOOLONG) {
		  return r;
	      }
	      j += r;
	      buf += strlen(buf);
	  }
	  else {
	      if((bufend-buf) < 5) {
		  return ETOOLONG;
	      }
	      strcpy(buf, "null");
	      buf += strlen(buf);
	  }
	  if((bufend-buf) < 1) {
	      return ETOOLONG;
	  }
	  buf[0] = ',';
	  buf++;
      }
      buf[-1] = '}';
      buf[0] = 0;
      return j + 1;
  } else if (t->type == JSMN_ARRAY) {
    // Min possible length is 2.
    if(maxlen < 3) {
	return ETOOLONG;
    }
    j = 0;
    buf[0] = '[';
    buf++;
    for (i = 0; i < t->size; i++) {
      int r = dumpjson(buf, bufend-buf, js, t + 1 + j, count - j);
      if(r == ETOOLONG) {
	  return r;
      }
      j += r;
      buf += strlen(buf);
      if((bufend-buf) < 1) {
	  return ETOOLONG;
      }
      buf[0] = ',';
      buf++;
    }
    buf[-1] = ']';
    buf[0] = 0;
    return j + 1;
  }
  return 0;
}

int yparse_get_function_msg(yparser *yp, char *topicbuf, int tb_max_len, char *valuebuf, int vb_max_len)
{
    /* Expect a dict like:
     * {water_meter/1: {litres: 123, direction: 1, tamper: 0}}
     *
     * I.e. a singleton dict with the name of the function as the key, and
     * the values as the subdict key/vals
     *
     * ->
     * water_meter/1/litres 123
     * water_meter/1/direction 1
     * water_meter/1/tamper 0
     *
     * This function acts like an iterator, returning each message in turn,
     * finally returning 0 to indicate no more messages.
     * This function expects the same topicbuf buffer to be passed at each
     * call, and will not rewrite the topic root if previously initialised.
     */
    
    // Expect no less than 5 tokens:
    // {<fn>: {<fnk1>: <fnv1>}}
    // 0 1    2  3       4
    
    if(yp->function_topic_root_token < 0) {
	// Not initialised.
	return 0;
    }
    if(yp->function_topic_root_len == 0) {
	// TODO: check topicbuf bounds
	jsmntok_t *fn_tok = yp->t + yp->function_topic_root_token + 1;
	yp->function_topic_root_len = fn_tok->end - fn_tok->start;
	strncpy(topicbuf, yp->data+fn_tok->start, yp->function_topic_root_len);
	topicbuf[yp->function_topic_root_len] = '/';
	yp->function_topic_root_len++;
	// Not terminating.
	yp->function_topic_next_attr_token = yp->function_topic_root_token + 3;
	yp->function_topic_next_attr_idx = 0;
    }
    jsmntok_t *attr_dict = yp->t + yp->function_topic_root_token + 2;

//    Log.notice(F("Fattr nexti=%d, nextt=%d, size=%d" CR), yp->function_topic_next_attr_idx, yp->function_topic_next_attr_token, attr_dict->size);
    // Return next attr/value pair
    if(yp->function_topic_next_attr_idx >= attr_dict->size) {
//	Log.notice(F("End of attributes" CR));
	return 0;
    }
    
    int n = yp->function_topic_next_attr_token;
    int kl = yp->t[n].end - yp->t[n].start;
//    Log.notice(F("Attr key: tok=%d start=%d end=%d" CR), n, yp->t[n].start, yp->t[n].end);
    strncpy(topicbuf+yp->function_topic_root_len, yp->data+yp->t[n].start, kl);
    topicbuf[yp->function_topic_root_len+kl] = 0;
    int incr = dumpjson(valuebuf, vb_max_len, yp->data, yp->t+n+1, yp->token_count-(n+1));
    if(incr < 0) {
	return incr;
    }
    yp->function_topic_next_attr_token += incr+1;
    yp->function_topic_next_attr_idx++;
    return 1;
}

int yparse_get_node_msg(yparser *yp, char *topicbuf, int tb_max_len, char *valuebuf, int vb_max_len)
{
    /* Expect a dict like:
     * {battv: 3123, uptime: 12345}
     *
     * ->
     * <gateway_base>/battv 3123
     * <gateway_base>/uptime 3123
     *
     * This function acts like an iterator, returning each message in turn,
     * finally returning 0 to indicate no more messages.
     */
    
    // Expect no less than 3 tokens:
    // {<fnk1>: <fnv1>}
    // 0  1       2
    
    if(yp->node_topic_root_token < 0) {
	// Not initialised.
	return 0;
    }
    jsmntok_t *attr_dict = yp->t + yp->node_topic_root_token;

    // Return next attr/value pair
    Log.trace(F("Nattr nexti=%d, nextt=%d, size=%d" CR), yp->node_topic_next_attr_idx, yp->node_topic_next_attr_token, attr_dict->size);
    if(yp->node_topic_next_attr_idx >= attr_dict->size) {
	return 0;
    }
    
    int n = yp->node_topic_next_attr_token;
    int kl = yp->t[n].end - yp->t[n].start;
//    Log.notice(F("Attr key: tok=%d start=%d end=%d" CR), n, yp->t[n].start, yp->t[n].end);
    strncpy(topicbuf, yp->data+yp->t[n].start, kl);
    topicbuf[kl] = 0;
    Log.trace(F("topic=%s" CR), topicbuf);
    int incr = dumpjson(valuebuf, vb_max_len, yp->data, yp->t+n+1, yp->token_count-(n+1));
    if(incr < 0) {
	return incr;
    }
    yp->node_topic_next_attr_token += incr+1;
    yp->node_topic_next_attr_idx++;
    return 1;
}


int yparse(yparser *yp, const char *data)
{
    yp->data = data;
    jsmn_init(&yp->jp);
    Log.trace(F("Parse '%s' with %d tokens" CR), data, sizeof(yp->t) / sizeof(yp->t[0]));
    int r = jsmn_parse(
		   &yp->jp, data, strlen(data), yp->t,
		   sizeof(yp->t) / sizeof(yp->t[0])
		  );
    
    if(r <= 0) {
	return r;
    }
    yp->token_count = r;
    yp->function_topic_root_token = -1;
    yp->node_topic_root_token = -1;
    
    jsmntok_t *first_child = &yp->t[1];
    
    if(first_child->end - first_child->start == 1) {
	switch(data[first_child->start]) {
	case 'f':
	    yp->function_topic_root_token = 2;
	    yp->function_topic_root_len = 0;
	  break;
	case 'n':
	    yp->node_topic_root_token = 2;
	    yp->node_topic_next_attr_token = yp->node_topic_root_token + 1;
	    yp->node_topic_next_attr_idx = 0;
	}
    }
    return r;
}
