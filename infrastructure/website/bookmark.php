<?php
    session_name('PIBSESSION');
    session_start();

    if (isset($_POST['image'])) {
        $bookmark = $_POST['image'];

        if (file_exists($bookmark)) {
            if (!isset($_SESSION['bookmarks'])) {
                $_SESSION['bookmarks'] = array($bookmark);
            } else {
                array_push($_SESSION['bookmarks'], $bookmark);
            }
        }
    }
?>