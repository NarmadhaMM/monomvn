\name{rwish}
\alias{rwish}
\alias{Wishart}
\title{Draw from the Wishart Distribution}
\description{
  Random generation from the Wishart distribution.
}
  
\usage{
   rwish(v, S)
}

\arguments{
    \item{v}{degrees of freedom (scalar)}
    \item{S}{inverse scale matrix \eqn{(p \times p)}{(p x p)}}
}

\value{
  Returns generates one random draw from the distribution which is a
  \code{matrix} with the same dimensions as \code{S}
}

\details{
The mean of a Wishart random variable with \code{v} degrees of freedom
and inverse scale matrix \code{S} is \eqn{vS}{vS}.
}

\references{
  This was copied from the \pkg{MCMCpack} package
}

\examples{
draw <- rwish(3, matrix(c(1,.3,.3,1),2,2))
}
\keyword{distribution}



