const url = window.location.host;

function getAudio(audioname) {
    fetch( '/?musicname='+audioname )
        .then( response => response.json() )
        .then(json => {
            dataURL = "data:audio/mp3;base64," + json.data;                        
            document.getElementById("audio-player").src = dataURL;
            document.getElementById("delete-audio").name = audioname;
        } )
}

function loadPlaylist() {
    fetch( '/musics' )
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
    let file = document.getElementById("myFile").files[0];
    let formData = new FormData();
    formData.append("file",file);

    fetch( '/?musicname='+file.name,{method: "POST", body: formData} );
}

function deleteAudio() {
    console.log("Delete "+document.getElementById("delete-audio").name);
    fetch( '/?musicname='+document.getElementById("delete-audio").name,{method: "DELETE"} );
}
