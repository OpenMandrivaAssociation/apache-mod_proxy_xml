/********************************************************************
	 Copyright (c) 2004, WebThing Ltd
	 Author: Nick Kew <nick@webthing.com>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.
     
This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
      
*********************************************************************/


/********************************************************************
	Note to Users
 
	You are requested to register as a user, at
	http://apache.webthing.com/registration.html
 
	This entitles you to support from the developer.
	I'm unlikely to reply to help/support requests from
	non-registered users, unless you're paying and/or offering
	constructive feedback such as bug reports or sensible
	suggestions for further development.
 
	It also makes a small contribution to the effort
	that's gone into developing this work.
*********************************************************************/

/* End of Notices */


#define VERSION_STRING "proxy_xml/0.1"

/* apache */
#include <http_protocol.h>
#include <http_config.h>
#include <apr_strings.h>
#include <ap_provider.h>

#include "xmlns.h"

module AP_MODULE_DECLARE_DATA proxy_xml_module ;

const xml_char_t* (*xmlns_get_attr_name)(const xmlns_attr_t* attrs, int attnum);
const xml_char_t* (*xmlns_get_attr_val)(const xmlns_attr_t* attrs, int attnum);
int (*xmlns_get_attr_parsed)(const xmlns_attr_t* attrs, int attnum, parsedname*
		res);
void (*xmlns_suppress_output)(xmlns_public*, int onoff);

static void xmlns_funcs(void) {
  xmlns_get_attr_name = APR_RETRIEVE_OPTIONAL_FN(mod_xmlns_get_attr_name);
  xmlns_get_attr_val = APR_RETRIEVE_OPTIONAL_FN(mod_xmlns_get_attr_val);
  xmlns_get_attr_parsed = APR_RETRIEVE_OPTIONAL_FN(mod_xmlns_get_attr_parsed);
  xmlns_suppress_output = APR_RETRIEVE_OPTIONAL_FN(mod_xmlns_suppress_output);
}


static const char* WMLNS = "http://www.wapforum.org/2001/wml" ;
static const char* XHTMLNS = "http://www.w3.org/1999/xhtml" ;

typedef struct urlmap {
  struct urlmap* next ;
  const char* from ;
  const char* to ;
} urlmap ;

typedef struct {
  urlmap* map ;
} proxy_xml_conf ;

typedef struct {
  const char* name ;
  const char** attrs ;
} elt_t ;

typedef struct {
  const char** empty ;
  const elt_t* elts ;
  proxy_xml_conf* cfg ;
} dtdecl ;

typedef struct {
  dtdecl* wmldecl;
  dtdecl* xhtmldecl;
} req_ctx;

static void xmlns_set_appdata(xmlns_public* ctx, const char* ns, void* ptr) {
  req_ctx* rctx = ap_get_module_config(ctx->f->r->request_config, &proxy_xml_module);
  if (rctx == NULL) {
    rctx = apr_pcalloc(ctx->f->r->pool, sizeof(req_ctx));
    ap_set_module_config(ctx->f->r->request_config, &proxy_xml_module, rctx);
  }
  if (ns == WMLNS) {
    rctx->wmldecl = ptr;
  }
  else if (ns == XHTMLNS) {
    rctx->xhtmldecl = ptr;
  }
}
static void* xmlns_get_appdata3(xmlns_public* ctx, const parsedname* name) {
  req_ctx* rctx = ap_get_module_config(ctx->f->r->request_config, &proxy_xml_module);
  if (rctx != NULL) {
    if (!strncmp(name->ns, WMLNS, name->nslen)) {
      return rctx->wmldecl;
    }
    else if (!strncmp(name->ns, XHTMLNS, name->nslen)) {
      return rctx->xhtmldecl;
    }
  }
  return NULL;
}
static void init_wml(xmlns_public* ctx, const xml_char_t* prefix,
	const xml_char_t* namesp ) {
  static const char* empty_elts[] = {
    "img" ,
    "timer" ,
    "br" ,
    "access" ,
    "meta" ,
    "noop" ,
    "postfield" ,
    "setvar" ,
    "input" ,
    NULL
  } ;
  static const char* href[] = { "href", NULL } ;
  static const char* src[] = { "src", NULL } ;
  static const char* onpick[] = { "onpick", NULL } ;
  static const char* cardev[] = { "onenterforward", "onenterbackward", "ontimer", NULL } ;

  static const elt_t linked_elts[] = {
    { "a" , href } ,
    { "go" , href } ,
    { "option" , onpick } ,
    { "img", src } ,
    { "link" , href } ,
    { "card" , cardev } ,
    { "template" , cardev } ,
    { NULL, NULL }
  } ;
  dtdecl* decl = apr_palloc(ctx->f->r->pool, sizeof(dtdecl)) ;
  decl->empty = empty_elts ;
  decl->elts = linked_elts ;
  decl->cfg = ap_get_module_config(ctx->f->r->per_dir_config,
	&proxy_xml_module) ;
  xmlns_set_appdata(ctx, WMLNS, decl) ;

}
static void init_xhtml(xmlns_public* ctx, const xml_char_t* prefix,
	const xml_char_t* namesp ) {
  static const char* empty_elts[] = {
    "br" ,
    "link" ,
    "img" ,
    "hr" ,
    "input" ,
    "meta" ,
    "base" ,
    "area" ,
    "param" ,
    "col" ,
    "frame" ,
    "isindex" ,
    "basefont" ,
    NULL
  } ;

  static const char* href[] = { "href", NULL } ;
  static const char* cite[] = { "cite", NULL } ;
  static const char* action[] = { "action", NULL } ;
  static const char* imgattr[] = { "src", "longdesc", "usemap", NULL } ;
  static const char* inputattr[] = { "src", "usemap", NULL } ;
  static const char* scriptattr[] = { "src", "for", NULL } ;
  static const char* frameattr[] = { "src", "longdesc", NULL } ;
  static const char* objattr[] = { "classid", "codebase", "data", "usemap", NULL } ;
  static const char* profile[] = { "profile", NULL } ;
  static const char* background[] = { "background", NULL } ;
  static const char* codebase[] = { "codebase", NULL } ;

  static const elt_t linked_elts[] = {
    { "a" , href } ,
    { "img" , imgattr } ,
    { "form", action } ,
    { "link" , href } ,
    { "script" , scriptattr } ,
    { "base" , href } ,
    { "area" , href } ,
    { "input" , inputattr } ,
    { "frame", frameattr } ,
    { "iframe", frameattr } ,
    { "object", objattr } ,
    { "q" , cite } ,
    { "blockquote" , cite } ,
    { "ins" , cite } ,
    { "del" , cite } ,
    { "head" , profile } ,
    { "body" , background } ,
    { "applet", codebase } ,
    { NULL, NULL }
  } ;
  dtdecl* decl = apr_palloc(ctx->f->r->pool, sizeof(dtdecl)) ;
  decl->empty = empty_elts ;
  decl->elts = linked_elts ;
  decl->cfg = ap_get_module_config(ctx->f->r->per_dir_config,
	&proxy_xml_module) ;
  xmlns_set_appdata(ctx, XHTMLNS, decl) ;
}


static int is_empty_elt(const parsedname* name3, const char** empty) {
  const char** p = empty ;
  while ( *p )
    if ( strncmp( *p, name3->elt, strlen(*p)) )
      ++p ;
    else
      return 1 ;
  return 0 ;
}

static int xend_element(xmlns_public* ctx, const parsedname* name3) {
  dtdecl* decl = xmlns_get_appdata3(ctx, name3) ;
  if ( is_empty_elt(name3, decl->empty) )
    return OK ;
  else
    return DECLINED ;
}
static int xstart_element(xmlns_public* ctx,
	const parsedname* name3, const xmlns_attr_t* attrs ) {

  dtdecl* decl = xmlns_get_appdata3(ctx, name3) ;

  int i ;
  const char** linkattrs ;
  const char** linkattr ;
  const elt_t* elt ;
  const char* value ;
  const char* name ;
  int is_uri ;
  urlmap* m ;

  if ( ! attrs ) {
    return DECLINED ;
  }
  linkattrs = 0 ;
  for ( elt = decl->elts;  elt->name != NULL ; ++elt )
    if ( !strncmp(elt->name, name3->elt, name3->eltlen) ) {
      linkattrs = elt->attrs ;
      break ;
    }
  if ( ! linkattrs ) {
    return DECLINED ;
  }

  ap_fputc(ctx->f->next, ctx->bb, '<') ;
  if ( name3->nparts == 3 ) {
    ap_fwrite(ctx->f->next, ctx->bb, name3->prefix, name3->prefixlen) ;
    ap_fputc(ctx->f->next, ctx->bb, ':') ;
  }
  ap_fwrite(ctx->f->next, ctx->bb, name3->elt, name3->eltlen) ;

  for ( i = 0 ; ; ++i ) {
/* Needto deal with prefixes here */
    name = xmlns_get_attr_name(attrs, i) ;
    if ( ! name )
      break ;
    value = xmlns_get_attr_val(attrs, i) ;
    if ( linkattrs && value ) {
      is_uri = 0 ;
      linkattr = linkattrs ;
      do {
	if ( !strcmp(*linkattr, name) ) {
	  is_uri = 1 ;
	  break ;
	}
      } while ( *++linkattr ) ;
      if ( is_uri ) {
	for ( m = decl->cfg->map ; m ; m = m->next ) {
	  if ( ! strncasecmp(value, m->from, strlen(m->from) ) ) {
	    value = apr_pstrcat(ctx->f->r->pool, m->to,
		value+strlen(m->from) , NULL) ;
	    break ;
	  }
	}
      }
    }
    if ( value ) {
      ap_fputstrs(ctx->f->next, ctx->bb, " ", name, "=\"",
		value, "\"", NULL) ;
    }
  }

  if ( is_empty_elt(name3, decl->empty) )
    ap_fputs(ctx->f->next, ctx->bb, " />") ;
  else
    ap_fputc(ctx->f->next, ctx->bb, '>') ;
  return OK ;
}

static const char* set_urlmap(cmd_parms* cmd, void* CFG,
        const char* from, const char* to) {
  proxy_xml_conf* cfg = (proxy_xml_conf*)CFG ;
  urlmap* newmap = apr_palloc(cmd->pool, sizeof(urlmap) ) ;
  newmap->from = apr_pstrdup(cmd->pool, from) ;
  newmap->to = apr_pstrdup(cmd->pool, to) ;
  newmap->next = NULL ;
  if ( cfg->map ) {
    urlmap* m = cfg->map ;
    while ( m->next )
        m = m->next ;
    m->next = newmap ;
  } else
    cfg->map = newmap ;
  return NULL ;
}
static const command_rec proxy_xml_cmds[] = {
  AP_INIT_TAKE2("ProxyXMLURLMap", set_urlmap, NULL,
	RSRC_CONF|ACCESS_CONF, "Map URL From To" ) ,
  { NULL }
} ;
static int mod_proxy_xml(apr_pool_t* p, apr_pool_t* p1, apr_pool_t* p2,
	server_rec* s) {
  ap_add_version_component(p, VERSION_STRING) ;
  return OK ;
}

static const xmlns xmlns_proxy_xhtml = {
	XMLNS_VERSION,
	xstart_element ,
	xend_element ,
	init_xhtml ,
	NULL ,
	NULL ,
	NULL ,
	NULL ,
	NULL
} ;
static const xmlns xmlns_proxy_wml = {
	XMLNS_VERSION,
	xstart_element ,
	xend_element ,
	init_wml ,
	NULL ,
	NULL ,
	NULL ,
	NULL
} ;
static void proxy_xml_hooks(apr_pool_t* p) {
  ap_register_provider(p, "xmlns", XHTMLNS, "proxy", &xmlns_proxy_xhtml) ;
  ap_register_provider(p, "xmlns", WMLNS, "proxy", &xmlns_proxy_wml) ;
  ap_hook_post_config(mod_proxy_xml, NULL, NULL, APR_HOOK_MIDDLE) ;
  ap_hook_optional_fn_retrieve(xmlns_funcs, NULL, NULL, APR_HOOK_MIDDLE) ;
}
static void* proxy_xml_config(apr_pool_t* pool, char* x) {
  proxy_xml_conf* ret = apr_pcalloc(pool, sizeof(proxy_xml_conf) ) ;
  return ret ;
}
static void* proxy_xml_merge(apr_pool_t* pool, void* BASE, void* ADD) {
  proxy_xml_conf* base = (proxy_xml_conf*) BASE ;
  proxy_xml_conf* add = (proxy_xml_conf*) ADD ;
  proxy_xml_conf* conf = apr_palloc(pool, sizeof(proxy_xml_conf)) ;
  if ( add->map && base->map ) {
    urlmap* a ;
    conf->map = NULL ;
    for ( a = base->map ; a ; a = a->next ) {
      urlmap* save = conf->map ;
      conf->map = apr_pmemdup(pool, a, sizeof(urlmap)) ;
      conf->map->next = save ;
    }
    for ( a = add->map ; a ; a = a->next ) {
      urlmap* save = conf->map ;
      conf->map = apr_pmemdup(pool, a, sizeof(urlmap)) ;
      conf->map->next = save ;
    }
  } else
    conf->map = add->map ? add->map : base->map ;
  return conf ;
} ;
module AP_MODULE_DECLARE_DATA proxy_xml_module = {
	STANDARD20_MODULE_STUFF,
	proxy_xml_config,
	proxy_xml_merge,
	NULL,
	NULL,
	proxy_xml_cmds,
	proxy_xml_hooks
} ;
