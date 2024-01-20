$$
\begin{align}
    \text{Program} & \to \text{\{Declaration\}} \\

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

    \text{DeclareVariable} & \to \text{Identifier ``:" }\text{Identifier} \\

    \text{DefineVariable} & \to \text{Identifier ``:" }\text{[Identifier] ``=" }\text{Expression} \\

    \text{NamedFunction} & \to \text{Identifier Function} \\

    \text{Function} & \to \text{``:" ``(" FunctionVariables ``)" Statement} \\

    \text{FunctionVariables} & \to
    \begin{cases}
        \text{[ReturnValues] [``;"]} \\
        \text{[ReturnValues] ``;" [Parameters]} \\
    \end{cases} \\

    \text{ReturnValues} & \to \text{DeclareVariable \{``," DeclareVariable\}} \\

    \text{Parameter} & \to \text{[ParameterPass] DeclareVariable} \\

    \text{Parameters} & \to \text{Parameter \{``," Parameter\}} \\

    \text{Scope} & \to \text{``\{" [ScopedStatement] ``\}"} \\

    \text{ScopedStatement} & \to
    \begin{cases}
        \text{Statement} \\
        \text{Declaration} \\
        \text{If \{Elif\}[Else]} \\
    \end{cases} \\

    \text{Statement} & \to
    \begin{cases}
        \text{Scope} \\
        \text{``return" ``;"} \\
        \text{Identifier ``=" Expression ``;"} \\
    \end{cases} \\

    \text{If} & \to \text{``if" Expression Statement} \\

    \text{Elif} & \to \text{``elif" Expression Statement} \\

    \text{Else} & \to \text{``else" Statement} \\

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

    \text{IntegerLiteral} & \to \text{\textbackslash d\{\textbackslash d\}} \\

    \text{Identifier} & \to \text{\textbackslash w\{(\textbackslash w|\textbackslash d)\}} \\
\end{align}
$$
