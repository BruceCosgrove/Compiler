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
\end{cases} \\
\text{[integer\_literal]} &\to \text{[0-9]}^* \\
\text{[identifier]} &\to \text{[a-zA-Z][a-zA-Z0-9]}^* \\

\end{align}
$$
