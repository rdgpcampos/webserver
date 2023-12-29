Web server for music player application.<br>

It supports the following operations:<br>
<ul>
  <li>Play audio file</li>
  <li>Upload new audio file</li>
  <li>Delete audio file</li>
  <li>Display list of audio files (playlist) to the client</li>
</ul>

The operations are implemented as follows:<br><br>
<b>Play audio file</b>: The client sends a GET HTTP request to the server with the audio file name as a parameter. The server responds with the audio file content. The client then loads the audio file on the audio control. <br><br>
<b>Upload new audio file</b>: The client submits a form containing the contents of a local audio file to the server using a POST HTTP request. The server receives the audio file content and creates a file of the same name in the playlist directory. <br><br>
<b>Delete audio file</b>: The client sends a DELETE HTTP request to the server with the audio file name as a parameter. The server searches for the file name in its playlist directory and deletes the file (if found).<br><br>
<b>Display list of audio files (playlist) to the client</b>: The client sends a GET HTTP request to the server. The server responds with the list of file names in its playlist directory. The client then displays these items in lexicographic order.<br><br>

The video below shows these operations at work.<br>

https://github.com/rdgpcampos/webserver/assets/70313745/9bc159c5-3372-4996-b09b-65c00746eac9

