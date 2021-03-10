<?php
    session_name('PIBSESSION');
    session_start();
?>
<!DOCTYPE html>
<html>
    <?php include 'header.php' ?>
    <body class="wrapper">
        <?php include 'nav.php' ?>
        <div class="main gallery">
        <?php
            if (isset($_SESSION['bookmarks'])) {
                foreach($_SESSION['bookmarks'] as $image) {
                    echo '<img src="' . $image . '" />';
                }
            } else {
                echo '<h2>Nemáte žiadne obľúbené obrázky :(</h2>';   
            }
        ?>
        </div> 
    </body>
</html>