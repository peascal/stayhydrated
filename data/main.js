const button = document.querySelector('#flex');

button.addEventListener('click', () => {
    let request = new Request('/hydrate?plant=1');
    fetch(request)
        .then((response) => {
            if (response.ok === false) {
                // Fehler
            }

            return response.json();
        })
        .then((res) => {
            console.log(res);
        })
})
