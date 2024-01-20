$$
\begin{align}
    \text{Program} & \to
        \text{\{Declaration\}} \\

    \text{Declaration} & \to
    \begin{cases}
        \text{Definition} \\
        \text{DeclareVariable ``;"} \\
    \end{cases} \\

    \text{Definition} & \to
    \begin{cases}
        \text{DefineVariable ``;"} \\
        \text{NamedFunction ``;"} \\
    \end{cases} \\

    \text{DeclareVariable} & \to
        \text{Identifier ``:" Identifier} \\

    \text{DefineVariable} & \to
        \text{Identifier ``:" [Identifier] ``=" Expression} \\

    \text{NamedFunction} & \to
        \text{Identifier Function} \\

    \text{Function} & \to
        \text{``:" ``(" FunctionVariables ``)" Statement} \\

    \text{FunctionVariables} & \to
    \begin{cases}
        \text{[ReturnValues] [``;"]} \\
        \text{[ReturnValues] ``;" [Parameters]} \\
    \end{cases} \\

    \text{ReturnValues} & \to
        \text{DeclareVariable \{``," DeclareVariable\}} \\

    \text{Parameters} & \to
        \text{Parameter \{``," Parameter\}} \\

    \text{Parameter} & \to
        \text{[ParameterPass] DeclareVariable} \\

    \text{Scope} & \to
        \text{``\{" \{ScopedStatement\} ``\}"} \\

    \text{ScopedStatement} & \to
    \begin{cases}
        \text{Statement} \\
        \text{Declaration} \\
        \text{If \{Elif\} [Else]} \\
        \text{While \{Elif\} [Else]} \\
    \end{cases} \\

    \text{Statement} & \to
    \begin{cases}
        \text{``;"} \\
        \text{Scope} \\
        \text{If} \\
        \text{``return" ``;"} \\
        \text{Identifier ``=" Expression ``;"} \\
    \end{cases} \\

    \text{If} & \to
        \text{``if" ``(" [DeclareOrDefineVariables ``;"] Expression ``)" Statement} \\

    \text{Elif} & \to
        \text{``elif" ``(" [DeclareOrDefineVariables ``;"] Expression ``)" Statement} \\

    \text{Else} & \to
        \text{``else" Statement} \\

    \text{While} & \to
        \text{(``dowhile" | ``while") ``(" [DeclareOrDefineVariables] ``;" [Expression] ``;" [Expressions] ``)" Statement} \\

    \text{DeclareOrDefineVariables} & \to
        \text{DeclareOrDefineVariable \{``," DeclareOrDefineVariable\}} \\

    \text{DeclareOrDefineVariable} & \to
        \text{(DeclareVariable | DefineVariable)} \\

    \text{Expressions} & \to
        \text{Expression \{``," Expression\}} \\

    \text{Expression} & \to
    \begin{cases}
        \text{Term} \\
        \text{Expression Operator Expression} \\
    \end{cases} \\

    \text{Term} & \to
    \begin{cases}
        \text{IntegerLiteral} \\
        \text{Identifier} \\
        \text{``(" Expression ``)"} \\
    \end{cases} \\

    \text{Operator} & \to
    \begin{cases}
        \text{``/"} & \text{precedence 1} \\
        \text{``\%"} & \text{precedence 1} \\
        \text{``*"} & \text{precedence 1} \\
        \text{``+"} & \text{precedence 0} \\
        \text{``-"} & \text{precedence 0} \\
    \end{cases} \\

    \text{ParameterPass} & \to
    \begin{cases}
        \text{``in"} & \text{default} \\
        \text{``out"} \\
        \text{``inout"} \\
        \text{``copy"} \\
        \text{``move"} \\
    \end{cases} \\

    \text{IntegerLiteral} & \to
        \text{\textbackslash d\{\textbackslash d\}} \\

    \text{Identifier} & \to
        \text{\textbackslash w\{(\textbackslash w|\textbackslash d)\}} \\
\end{align}
$$
