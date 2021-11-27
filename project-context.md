# Project Questions
## BNF Grammar
```
<program>     ->     <vars> program <block>
<block>       ->     start <vars> <stats> stop
<vars>        ->     empty | declare Identifier =  Integer  ;  <vars>
<expr>        ->     <N> + <expr>  | <N>
<N>           ->     <A> / <N> | <A> * <N> | <A>
<A>           ->     <M> - <A> | <M>
<M>           ->     . <M> |  <R>
<R>           ->      ( <expr> ) | Identifier | Integer
<stats>       ->      <stat>  <mStat>
<mStat>       ->      empty |  <stat>  <mStat>
<stat>        ->      <in> ;  | <out> ;  | <block> | <if> ;  | <loop> ;  | <assign> ;
                              | <goto> ; | <label> ;
<in>          ->      listen  Identifier
<out>         ->      talk <expr>
<if>          ->      if [ <expr> <RO> <expr> ] then <stat>
                        | if [ <expr> <RO> <expr> ] then <stat> else <stat>
<loop>        ->      while  [ <expr> <RO> <expr> ]  <stat>
<assign>      ->      assign Identifier  = <expr>
<RO>          ->       >  | < |  ==  |   { == }  (three tokens)  | %
<label>       ->    label Identifier
<goto>        ->    jump Identifier
```

## Code Gen
- Could do single top statement as first test
