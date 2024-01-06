$$
\begin{align}
    \text{[program]} &\to \text{[declaration]}^* \\

    \text{[declaration]} &\to
    \begin{cases}
        \text{[definition]} \\
        \text{[declare\_variable]} \\
    \end{cases} \\

    \text{[definition]} &\to
    \begin{cases}
        \text{[define\_variable]} \\
        \text{[define\_function]} \\
    \end{cases} \\

    \text{[declare\_variable]} &\to\space:let \\

    \text{[define\_variable]} &\to\space:let=\text{[expression]} \\

    \text{[parameter]} &\to \text{[parameter\_pass]}?\space\text{[identifier]}:let \\

    \text{[define\_function]} &\to
    \begin{cases}
        :\text{([variable\_declarations]?;?)}\text{[statement]} \\
        :\text{([variable\_declarations]?;[variable\_declarations])}\text{[statement]} \\
    \end{cases} \\

    \text{[scope]} &\to \{\text{[scoped\_statement]}^*\} \\

    \text{[scoped\_statement]} &\to
    \begin{cases}
        \text{[statement]} \\
        \text{[identifier]}\text{[declaration]}; \\
        if\space\text{[expression]}\text{[statement]} \\
    \end{cases} \\

    \text{[statement]} &\to
    \begin{cases}
        \text{[scope]} \\
        return\space\text{[expression]}; \\
    \end{cases} \\

    \text{[expression]} &\to
    \begin{cases}
        \text{[term]} \\
        \text{[expression]}\text{[binary\_operator]}\text{[expression]} \\
    \end{cases} \\

    \text{[term]} &\to
    \begin{cases}
        \text{[integer\_literal]} \\
        \text{[identifier]} \\
        \text{([expression])} \\
    \end{cases} \\

    \text{[binary\_operator]} &\to
    \begin{cases}
        \text{/} & \text{precedence 1} \\
        \text{\%} & \text{precedence 1} \\
        \text{*} & \text{precedence 1} \\
        \text{+} & \text{precedence 0} \\
        \text{-} & \text{precedence 0} \\
    \end{cases} \\

    \text{[parameter\_pass]} &\to
    \begin{cases}
        in & \text{default} \\
        out \\
        inout \\
        copy \\
        move \\
    \end{cases} \\

    \text{[integer\_literal]} &\to [0-9]^* \\

    \text{[identifier]} &\to [a-zA-Z\_][a-zA-Z0-9\_]^* \\
\end{align}
$$
