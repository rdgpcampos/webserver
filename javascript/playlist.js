function getAudio(audioname) {
    fetch( 'http://127.0.0.1:5050/?musicname='+audioname )
        .then( response => response.json() )
        .then(json => {
            dataURL = "data:audio/mp3;base64," + json.data;                        
            document.getElementById("audio-player").src = dataURL;
        } )
}

function loadPlaylist() {
    fetch( 'http://127.0.0.1:5050/musics' )
        .then( response => response.json() )
        .then( json => {
            // load json response to a list property that will populate the dropdown
            var dropdown = document.getElementById("myDropdown");
            

            if (dropdown.childElementCount != json.length) {
                dropdown.innerHTML = ''; // clean list
                for (let i = 0; i < json.length; i++) {
                    var elem = document.createElement("button")
                    dropdown.append(elem);
                    elem.innerHTML = "<button onclick='getAudio(\""+ json[i].name +"\")'>"+ json[i].name +"</button>";
                }
            }


            document.getElementById("myDropdown").classList.toggle("show");
        } )
}

// Close the dropdown if the user clicks outside of it
window.onclick = function(event) {
    if (!event.target.matches('.dropbtn')) {
        var dropdowns = document.getElementsByClassName("dropdown-content");
        
        for (let i = 0; i < dropdowns.length; i++) {
        var openDropdown = dropdowns[i];
        if (openDropdown.classList.contains('show')) {
            openDropdown.classList.remove('show');
        }
        }
    }
}

// send file to server
function sendFileToServer() {
    console.log("Submitting form");
    let file = document.getElementById("myFile").files[0];
    let formData = new FormData();
    formData.append("file",file);

    fetch('http://127.0.0.1:5050/',{method: "POST", body: file});
}
