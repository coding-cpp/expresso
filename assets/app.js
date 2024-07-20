document.addEventListener('DOMContentLoaded', async function () {
  const response = await fetch('/about')
  response.headers.forEach((value, name) => {
    if (name === 'expresso') {
      document.querySelector('i').innerHTML = value
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
})
