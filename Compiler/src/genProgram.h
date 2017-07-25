/* ///////////////////////////////////////////////////////////////////////////

  Copyright 2015-2017 Christopher Bak

  This file is part of the GP 2 Compiler. The GP 2 Compiler is free software: 
  you can redistribute it and/or modify it under the terms of the GNU General
  Public License as published by the Free Software Foundation, either version 3
  of the License, or (at your option) any later version.

  The GP 2 Compiler is distributed in the hope that it will be useful, but 
  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY 
  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for 
  more details.

  You should have received a copy of the GNU General Public License
  along with the GP 2 Compiler. If not, see <http://www.gnu.org/licenses/>.

  =======================
  Generate Program Module
  =======================    

  Generates the main function of the runtime system, which processes the host
  graph and calls the functions to apply and match rules according to the
  control constructs of the GP 2 program.

/////////////////////////////////////////////////////////////////////////// */

#ifndef INC_GEN_PROGRAM_H
#define INC_GEN_PROGRAM_H

#include "ast.h"
#include "common.h"

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdlib.h> 
#include <stdio.h> 
#include <string.h> 

void generateRuntimeMain(List *declarations, string output_dir);

/* Each GP 2 control construct is translated into a fragment of C code. 
 * I give the "broad strokes" translation here, excluding the more fiddly
 * details such as the management of graph backtracking. The runtime code
 * has a global boolean variable 'success' whose value is set to control
 * program flow.
 *
 * The bulk of the generated code comes from rule calls and failure to match
 * a rule, The rest of the control constructs translate to C control structures
 * wrapped around rule calls and failures.
 *
 * Rule Call (R)
 * =============
 * There are several cases depending on the structure of R. 
 * (1) R has no LHS: 
 * applyR();
 * success = true;
 *
 * (2) R does not change the host graph:
 * if(matchR(M_R))
 * {
 *    initialiseMorphism(M_R);
 *    success = true;
 * }
 * else
 * {
      <context-dependent failure code>
 * }
 *
 * (3) R has a non-empty LHS and changes the host graph:
 * if(matchR(M_R))
 * {
 *    applyR(M_R);
 *    initialiseMorphism(M_R);
 *    success = true;
 * }
 * else
 * {
      <context-dependent failure code>
 * }
 *
 * Notes: 
 * - The functions to apply a rule take a boolean argument to signal whether
 *   graph changes should be recorded. These are omitted here. 
 * - M_R is the name of the morphism structure associated with rule R.
 * - For empty-LHS rules, the generated rule application function takes no
 *   morphism argument.
 * - initialiseMorphism resets the values of the morphism to their default
 *   valuess, so that matches of a rule are not influenced by values from a
 *   previous match attempt. 
 *
 * Failure Code
 * ============
 * The code generated for a failure to match a rule depends on the context of
 * the rule call.
 *
 * Failure code for rules at the 'top level' is:
 * fprintf(output_file, "No output graph: rule <rule_name> not applicable.\n");
 * OR
 * fprintf(outout_file, "No output graph: Fail statement invoked.\n");
 * garbageCollect();
 * return 0;
 *
 * Failure code for rules within a loop body is:
 * success = false;
 * <host graph restoration code>
 *
 * Failure code for rules within a branch condition is:
 * success = false;
 * break; (branch conditions are generated in a do-while loop)
 *
 *
 * Rule Set Call {R1, R2}
 * ==========================
 * The rules are executed sequentially within a do-while loop so that execution
 * execution can jump outside the rule set if a rule match succeeds before the
 * end of the rule set.
 *
 * do
 * {
 *    if(matchR1(M_R1))
 *    {
 *       <matching success code>
 *       break;
 *    }
 *
 *    if(matchR2(M_R2))
 *    {
 *       <matching success code>
 *    }
 *    else <context-dependent failure code>
 * } while(false);
 *
 *
 * Conditional Branch if/try C then P else Q
 * ===========================================
 * If statements and try statements generate the same code to restore the host
 * graph to its state before the condition was entered. The condition program 
 * is generated within a do-while-false loop so the subprogram can be exited,
 * using a C 'break' statement, as soon as possible after failure detection.
 *
 * do
 * {
 *    <program code for C>
 * } while(false);
 *
 * <host graph restoration code for if statements>
 *
 * if(success)
 * {
 *    <program code for P>
 * }
 * else
 * {
 *    <host graph restoration code for try statements>
 *    <program code for Q>
 * }
 *
 *
 * Loop Statement P!
 * =================
 * Nothing special here, just a direct translation to a C loop:
 * while(success)
 * {
 *    <program code for P>
 * }
 * The program code will set the success flag to false when a rule application
 * fails (in some contexts) which will break the loop.
 *
 * Or Statement P or Q
 * ===================
 * C's rand function is used to nondeterministically choose between the two programs.
 *
 * int random = rand();
 * if((random %% 2) == 0)
 * {
 *    <program code for P>
 * }
 * else
 * {
 *    <program code for Q>
 * }
 *   
 * Skip, Fail and Break
 * ====================
 * 'skip' => success = true;
 * 'fail' => <context-dependent failure code>
 * 'break' => <code to handle graph backtracking>; break;
 */


#endif /* INC_GEN_PROGRAM_H */
