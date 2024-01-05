$$
\begin{align}
    \text{[program]} &\to \text{[statement]}^* \\
    \text{[statement]} &\to
    \begin{cases}
        return\space\text{[expression]}; \\
        let\space\text{[identifier]}=\text{[expression]}; \\
    \end{cases} \\
    \text{[expression]} &\to
    \begin{cases}
        \text{[integer\_literal]} \\
        \text{[identifier]} \\
        \text{[binary\_expression]} \\
    \end{cases} \\
    \text{[binary\_expression]} &\to \text{[expression]}\text{[binary\_operator]}\text{[expression]} \\
    \text{[integer\_literal]} &\to [0-9]^* \\
    \text{[identifier]} &\to [a-zA-Z][a-zA-Z0-9]^* \\
    \text{[binary\_operator]} &\to
    \begin{cases}
        \text{*} & \text{precedence 1} \\
        \text{/} & \text{precedence 1} \\
        \text{\%} & \text{precedence 1} \\
        \text{+} & \text{precedence 2} \\
        \text{-} & \text{precedence 2} \\
    \end{cases} \\
\end{align}
$$
