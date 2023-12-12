// test script
function getAudio(audioname) {
    fetch( 'http://127.0.0.1/?musicname='+audioname )
        .then( response => {
            console.log(response.body)
        } );
}