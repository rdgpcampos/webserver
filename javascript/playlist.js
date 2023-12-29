const url = window.location.host;
var background_toggled = 0;

function getAudio(audioname) {
    fetch( '/?musicname='+audioname )
        .then( response => response.json() )
        .then(json => {
            dataURL = "data:audio/mp3;base64," + json.data;                        
            document.getElementById("audio-player").src = dataURL;
        } )
}

function toggleModal(toggle_flag) {
    var modal = document.getElementById('modal');
    var backdrop = document.getElementById('backdrop');

    if (background_toggled == 0 && toggle_flag) {
        modal.style.display = "inline-block";
        backdrop.style.background = "#33333377";
        backdrop.style.backdropFilter = "blur(1px)";
        backdrop.style.zIndex = "9998";
        background_toggled = 1;
    } else if (background_toggled == 1 && !toggle_flag) {
        modal.style.display = "";
        backdrop.style.background = "#ffffff00";
        backdrop.style.backdropFilter = "blur(0)";
        backdrop.style.zIndex = "-1";
        background_toggled = 0;
    }
}

function loadPlaylist() {
    fetch( '/musics' )
        .then( response => response.json() )
        .then( json => {
            // load json response to a list property that will populate the dropdown
            var dropdown = document.getElementById("playlist-dropdown");

            if (dropdown.childElementCount != json.length) {
                dropdown.innerHTML = ''; // clean list
                for (let i = 0; i < json.length; i++) {
                    var elem = document.createElement("div")
                    dropdown.append(elem);
                    elem.style.display = "inline-block";
                    elem.style.verticalAlign = "center";
                    elem.style.width = "300px";
                    elem.innerHTML = "<button onclick='getAudio(\""+ json[i].name +"\")' class='song-option'>"+ json[i].name +"</button>";
                    elem.innerHTML += "<button onclick='deleteAudio(\""+json[i].name +"\")' class='song-delete-button'></button>";
                }
            }

            //document.getElementById("playlist-dropdown").classList.toggle("show");
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

    fetch( '/?musicname='+file.name,{method: "POST", body: formData} ).finally(() => {
        loadPlaylist();
    });

    toggleModal(false);
}

function deleteAudio(audioname) {
    //console.log("Delete "+document.getElementById("delete-audio").name);
    //fetch( '/?musicname='+document.getElementById("delete-audio").name,{method: "DELETE"} ).finally(() => {
    //    loadPlaylist();
    //});
    console.log("Delete "+audioname);
    fetch( '/?musicname='+audioname, {method: "DELETE"} ).finally( () => {
        loadPlaylist();
    } )
}
