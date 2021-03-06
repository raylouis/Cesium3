/*

Copyright 2012 William Hart. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are
permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of
      conditions and the following disclaimer.

   2. Redistributions in binary form must reproduce the above copyright notice, this list
      of conditions and the following disclaimer in the documentation and/or other materials
      provided with the distribution.

THIS SOFTWARE IS PROVIDED BY William Hart ``AS IS'' AND ANY EXPRESS OR IMPLIED
WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL William Hart OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/

#include <stdarg.h>
#include "parser.h"

extern ast_t * ast_nil;

combinator_t * new_combinator()
{
    combinator_t * c = GC_MALLOC(sizeof(combinator_t));

    c->fn = NULL;
    c->args = NULL;

    return c;
}

ast_t * match_fn(input_t * in, void * args)
{
    char * str = ((match_args *) args)->str;

    int start = in->start;
    int i = 0, len = strlen(str);
   
    skip_whitespace(in);
   
    while (i < len && str[i] == read1(in)) i++;
   
    if (i != len)
    {
       in->start = start;
       return NULL;
    }

    return ast_nil;
}

combinator_t * match(char * str)
{
    match_args * args = GC_MALLOC(sizeof(match_args));
    args->str = str;
    
    combinator_t * comb = new_combinator();
    comb->fn = match_fn;
    comb->args = args;

    return comb;
}

ast_t * expect_fn(input_t * in, void * args)
{
    expect_args * eargs = (expect_args *) args;
    ast_t * ast;
    
    combinator_t * comb = eargs->comb;

    if (ast = parse(in, comb))
       return ast;
    else
       exception(eargs->msg);

    return NULL;
}

combinator_t * expect(combinator_t * c, char * msg)
{
    expect_args * args = GC_MALLOC(sizeof(expect_args));
    args->msg = msg;
    args->comb = c;
    
    combinator_t * comb = new_combinator();
    comb->fn = expect_fn;
    comb->args = (void *) args;

    return comb;
}

ast_t * exact_fn(input_t * in, void * args)
{
    char * str = ((match_args *) args)->str;

    int start = in->start;
    int i = 0, len = strlen(str);
   
    while (i < len && str[i] == read1(in)) i++;
   
    if (i != len)
    {
       in->start = start;
       return NULL;
    }

    return ast_nil;
}

combinator_t * exact(char * str)
{
    match_args * args = GC_MALLOC(sizeof(match_args));
    args->str = str;
    
    combinator_t * comb = new_combinator();
    comb->fn = exact_fn;
    comb->args = args;

    return comb;
}

ast_t * range_fn(input_t * in, void * args)
{
    char * str = ((match_args *) args)->str;
    int start = in->start;

    char c = read1(in);

    if (str[0] <= c && str[1] >= c)
       return ast_nil;
    else
    {
       in->start = start;
       return NULL;
    }
}

combinator_t * range(char * str)
{
    match_args * args = GC_MALLOC(sizeof(match_args));
    args->str = str;
    
    if (strlen(str) != 2)
       exception("String not of length 2 in range\n");

    combinator_t * comb = new_combinator();
    comb->fn = range_fn;
    comb->args = args;

    return comb;
}

ast_t * alpha_fn(input_t * in, void * args)
{
    int start = in->start;

    char c = read1(in);

    if (isalpha(c))
       return ast_nil;
    else
    {
       in->start = start;
       return NULL;
    }
}

combinator_t * alpha()
{
    combinator_t * comb = new_combinator();
    comb->fn = alpha_fn;
    comb->args = NULL;

    return comb;
}

ast_t * digit_fn(input_t * in, void * args)
{
    int start = in->start;

    char c = read1(in);

    if (isdigit(c))
       return ast_nil;
    else
    {
       in->start = start;
       return NULL;
    }
}

combinator_t * digit()
{
    combinator_t * comb = new_combinator();
    comb->fn = digit_fn;
    comb->args = NULL;

    return comb;
}

ast_t * anything_fn(input_t * in, void * args)
{
    int start = in->start;

    char c = read1(in);

    return ast_nil;
}

combinator_t * anything()
{
    combinator_t * comb = new_combinator();
    comb->fn = anything_fn;
    comb->args = NULL;

    return comb;
}

ast_t * integer_fn(input_t * in, void * args)
{
   int start, len;
   char c, * text;

   ast_t * ast = new_ast();

   skip_whitespace(in);

   start = in->start;

   c = read1(in);

   if (!isdigit(c))
   {
      in->start = start;
      return NULL;
   }

   if (c == '0')
   {
      ast->typ = T_INT;
      
      ast->sym = sym_lookup("0");

      return ast;
   }

   while (isdigit(c = read1(in))) ;
   in->start--;

   ast->typ = T_INT;

   len = in->start - start;
   text = GC_MALLOC(len + 1);
        
   strncpy(text, in->input + start, len);
   text[len] = '\0';

   ast->sym = sym_lookup(text);

   return ast;
}

combinator_t * integer()
{
    combinator_t * comb = new_combinator();
    comb->fn = integer_fn;
    comb->args = NULL;

    return comb;
}

ast_t * cident_fn(input_t * in, void * args)
{
   int start, len;
   char c, * text;

   ast_t * ast = new_ast();

   skip_whitespace(in);

   start = in->start;

   c = read1(in);

   if (c != '_' && !isalpha(c))
   {
      in->start = start;
      return NULL;
   }
   
   while ((c = read1(in)) == '_' || isalpha(c) || isdigit(c)) ;
   in->start--;

   ast->typ = T_IDENT;

   len = in->start - start;
   
   text = GC_MALLOC(len + 1);
        
   strncpy(text, in->input + start, len);
   text[len] = '\0';

   ast->sym = sym_lookup(text);

   return ast;
}

combinator_t * cident()
{
    combinator_t * comb = new_combinator();
    comb->fn = cident_fn;
    comb->args = NULL;

    return comb;
}

seq_list * new_seq()
{
    return GC_MALLOC(sizeof(seq_list));
}

ast_t * seq_fn(input_t * in, void * args)
{
    int start = in->start;
    seq_args * sa = (seq_args *) args;
    seq_list * seq = sa->list;
    
    ast_t * ret = new_ast();
    ret->typ = sa->typ;

    ast_t * ptr = ret;

    while (seq != NULL)
    {
        ast_t * a = parse(in, seq->comb);
        if (a == NULL)
        {
           in->start = start;
           return NULL;
        }

        if (a != ast_nil)
        {
            ptr->next = a;
            ptr = ptr->next;
        }
        
        seq = seq->next;
    }

    if (sa->typ == T_NONE)
       return ret->next;
    else
    {
       ret->child = ret->next;
       ret->next = NULL;
       return ret;
    }
}

combinator_t * seq(combinator_t * ret, tag_t typ, combinator_t * c1, ...)
{
    combinator_t * comb;
    seq_list * seq;
    seq_args * args;

    va_list ap;
    va_start(ap, c1);

    seq = new_seq();
    seq->comb = c1;

    args = GC_MALLOC(sizeof(seq_args));
    args->typ = typ;
    args->list = seq;
    ret->args = (void *) args;
    ret->fn = seq_fn;

    while ((comb = va_arg(ap, combinator_t *)) != NULL)
    {
        seq->next = new_seq();
        seq = seq->next;
        seq->comb = comb;
    }

    va_end(ap);

    seq->next = NULL;

    return ret;
}

ast_t * multi_fn(input_t * in, void * args)
{
    seq_list * seq = ((seq_args *) args)->list;
    tag_t typ = ((seq_args *) args)->typ;

    while (seq != NULL)
    {
        ast_t * a = parse(in, seq->comb);
        if (a != NULL)
        {
           if (typ == T_NONE)
              return a;
           
           ast_t * res = new_ast();
           res->typ = typ;
           res->child = a;
           return res;
        }
        
        seq = seq->next;
    }

    return NULL;
}

combinator_t * multi(combinator_t * ret, tag_t typ, combinator_t * c1, ...)
{
    combinator_t * comb;
    seq_list * seq;
    seq_args * args;

    va_list ap;
    va_start(ap, c1);

    seq = new_seq();
    seq->comb = c1;

    args = GC_MALLOC(sizeof(seq_args));
    args->typ = typ;
    args->list = seq;
    
    ret->args = (void *) args;
    ret->fn = multi_fn;

    while ((comb = va_arg(ap, combinator_t *)) != NULL)
    {
        seq->next = new_seq();
        seq = seq->next;
        seq->comb = comb;
    }

    va_end(ap);

    seq->next = NULL;

    return ret;
}

ast_t * capture_fn(input_t * in, void * args)
{
    capture_args * cap = (capture_args *) args;
    
    int start;
    
    skip_whitespace(in);
   
    start = in->start;
    if (parse(in, cap->comb))
    {
        ast_t * a = new_ast();
        int len = in->start - start;
        char * text = GC_MALLOC(len + 1);
        
        strncpy(text, in->input + start, len);
        text[len] = '\0';

        a->typ = cap->typ;
        a->sym = sym_lookup(text);

        return a;
    }
    
    return NULL;
}

combinator_t * capture(tag_t typ, combinator_t * c)
{
    capture_args * args = GC_MALLOC(sizeof(capture_args));
    args->typ = typ;
    args->comb = c;
    
    combinator_t * comb = new_combinator();
    comb->fn = capture_fn;
    comb->args = args;

    return comb;
}

ast_t * not_fn(input_t * in, void * args)
{
   combinator_t * comb = (combinator_t *) args;
   int start = in->start;

   if (parse(in, comb))
   {
      in->start = start;
      return NULL;
   } else
      return ast_nil;
}

combinator_t * not(combinator_t * c)
{
    combinator_t * comb = new_combinator();
    comb->fn = not_fn;
    comb->args = (void *) c;

    return comb;
}

ast_t * option_fn(input_t * in, void * args)
{
   combinator_t * comb = (combinator_t *) args;
   ast_t * ast;
   int start = in->start;

   if (ast = parse(in, comb))
      return ast;
   else
      return ast_nil;
}

combinator_t * option(combinator_t * c)
{
    combinator_t * comb = new_combinator();
    comb->fn = option_fn;
    comb->args = (void *) c;

    return comb;
}

ast_t * zeroplus_fn(input_t * in, void * args)
{
   capture_args * cap = (capture_args *) args;
   combinator_t * comb = cap->comb;
   
   ast_t * ast;
   ast_t ** ptr = &ast;

   while ((*ptr) = parse(in, comb))
      ptr = &((*ptr)->next);
   
   if (ast == NULL)
      return ast_nil;
   else if (cap->typ == T_NONE)
      return ast;
   else
   {
      ast_t * res = new_ast();
      res->typ = cap->typ;
      res->child = ast;
      return res;
   }
}

combinator_t * zeroplus(tag_t typ, combinator_t * c)
{
    capture_args * args = GC_MALLOC(sizeof(capture_args));
    args->typ = typ;
    args->comb = c;
    
    combinator_t * comb = new_combinator();
    comb->fn = zeroplus_fn;
    comb->args = args;

    return comb;
}

ast_t * oneplus_fn(input_t * in, void * args)
{
   capture_args * cap = (capture_args *) args;
   combinator_t * comb = cap->comb;
   
   ast_t * ast;
   ast_t ** ptr = &ast;

   ast = parse(in, comb);
   if (!ast)
      return ast_nil;
   ptr = &(ast->next);
   
   while ((*ptr) = parse(in, comb))
      ptr = &((*ptr)->next);
   
   if (cap->typ == T_NONE)
      return ast;
   else
   {
      ast_t * res = new_ast();
      res->typ = cap->typ;
      res->child = ast;
      return res;
   }
}

combinator_t * oneplus(tag_t typ, combinator_t * c)
{
    capture_args * args = GC_MALLOC(sizeof(capture_args));
    args->typ = typ;
    args->comb = c;
    
    combinator_t * comb = new_combinator();
    comb->fn = oneplus_fn;
    comb->args = args;

    return comb;
}

ast_t * expr_fn(input_t * in, void * args)
{
   int alt;
   tag_t tag;
   op_t * op;
   expr_list * list = (expr_list *) args;

   if (list->fix == EXPR_BASE)
      return parse(in, list->comb);

   if (list->fix == EXPR_INFIX)
   {
      if (list->assoc == ASSOC_LEFT)
      {
         ast_t * lhs = expr_fn(in, (void *) list->next);
         if (!lhs)
            return NULL;
         
         while (1)
         {
            ast_t * rhs;
            
            op = list->op;
            while (op)
            {
               if (parse(in, op->comb))
                  break;
               op = op->next;
            }
            if (!op) break;

            rhs = expr_fn(in, (void *) list->next);
            if (!rhs)
               exception("Expression expected!\n");

            lhs = ast2(op->tag, lhs, rhs);
         }

         return lhs;
      } else if (list->assoc == ASSOC_RIGHT)
      {
         ast_t * lhs = expr_fn(in, (void *) list->next);
         ast_t ** ptr;

         if (!lhs)
            return NULL;
         
         ptr =  &lhs;

         while (1)
         {
            ast_t * rhs;
            
            op = list->op;
            while (op)
            {
               if (parse(in, op->comb))
                  break;
               op = op->next;
            }
            if (!op) break;

            rhs = expr_fn(in, (void *) list->next);
            if (!rhs)
               exception("Expression expected!\n");

            (*ptr) = ast2(op->tag, *ptr, rhs);
            ptr = &((*ptr)->child->next);
         }

         return lhs;
      } else
         exception("Invalid associativity for infix operator\n");
   } else if (list->fix == EXPR_PREFIX)
   {
      ast_t * rhs;
      
      op = list->op;
      while (op)
      {
         if (parse(in, op->comb))
            break;
         op = op->next;
      }
      
      rhs = expr_fn(in, (void *) list->next);
      if (op && !rhs)
         exception("Expression expected!\n");

      if (op)
         return ast1(op->tag, rhs);
      else
         return rhs;
   } else if (list->fix == EXPR_POSTFIX)
   {
      ast_t * lhs = expr_fn(in, (void *) list->next); 
      if (!lhs)
         return NULL;

      op = list->op;
      while (op)
      {
         if (parse(in, op->comb))
            break;
         op = op->next;
      }
      
      if (op)
         return ast1(op->tag, lhs);
      else
         return lhs;
   }
}

combinator_t * expr(combinator_t * exp, combinator_t * base)
{
   expr_list * args = GC_MALLOC(sizeof(expr_list));
   args->next = NULL;
   args->fix = EXPR_BASE;
   args->comb = base;

   exp->fn = expr_fn;
   exp->args = args;

   return exp;
}

void expr_insert(combinator_t * expr, int prec, tag_t tag, expr_fix fix, 
                 expr_assoc assoc, combinator_t * comb)
{
   expr_list * list = (expr_list *) expr->args;
   int i;

   expr_list * node = GC_MALLOC(sizeof(expr_list));
   op_t * op = GC_MALLOC(sizeof(op_t));

   op->tag = tag;
   op->comb = comb;
   node->op = op;
   node->fix = fix;
   node->assoc = assoc;
   
   if (prec == 0)
   {
      node->next = list;
      expr->args = (void *) node;
      return;
   }
   
   for (i = 0; list != NULL && i < prec - 1; i++)
      list = list->next;

   if (list->fix == EXPR_BASE || list == NULL)
      exception("Invalid precedence for expression\n");

   node->next = list->next;
   list->next = node;
}

void expr_altern(combinator_t * expr, int prec, tag_t tag, combinator_t * comb)
{
   op_t * op = GC_MALLOC(sizeof(op_t));
   expr_list * list = (expr_list *) expr->args;
   int i;

   for (i = 0; list != NULL && i < prec; i++)
      list = list->next;

   if (list->fix == EXPR_BASE || list == NULL)
      exception("Invalid precedence for expression\n");

   op->tag = tag;
   op->comb = comb;
   op->next = list->op;
   list->op = op;
}

ast_t * parse(input_t * in, combinator_t * comb)
{
    return comb->fn(in, (void *)comb->args);
}
