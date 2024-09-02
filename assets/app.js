const about = async () => {
  const response = await fetch('/about')
  response.headers.forEach((value, name) => {
    if (name === 'expresso') {
      document.getElementById('version').innerHTML = value
    }
  })

  const data = await response.json()
  const orderedList = document.querySelector('ol')
  data?.submodules.forEach(submodule => {
    const listItem = document.createElement('li')
    const a = document.createElement('a')
    a.href = submodule.repository
    a.innerText = submodule.name
    a.target = '_blank'
    const span = document.createElement('span')
    span.innerText = ': ' + submodule.work
    listItem.appendChild(a)
    listItem.appendChild(span)
    orderedList.appendChild(listItem)
  })

  const creator = document.getElementById('creator')
  creator.innerText = data?.creator?.name
  creator.href = data?.creator?.github
  creator.target = '_blank'

  document.querySelectorAll('.github').forEach(element => {
    element.href = data?.repository
    element.target = '_blank'
  })
}

const contributors = async () => {
  const response = await fetch(
    'https://api.github.com/repos/coding-cpp/expresso/contributors'
  )
  const data = await response.json()

  const contributorsList = document.querySelector('ul')
  data.forEach(contributor => {
    // for (let i = 0; i < 100; i++) {
    const listItem = document.createElement('li')
    const a = document.createElement('a')
    const img = document.createElement('img')
    a.href = contributor.html_url
    a.target = '_blank'
    img.src = contributor.avatar_url
    a.appendChild(img)
    listItem.appendChild(a)
    contributorsList.appendChild(listItem)
    // }
  })
}

document.addEventListener('DOMContentLoaded', about)
document.addEventListener('DOMContentLoaded', contributors)
