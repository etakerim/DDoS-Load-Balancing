<!DOCTYPE html>
<html>
    <?php include 'header.php' ?>
    <body class="wrapper">
        <?php include 'nav.php' ?>
        <div class="main">
            <h2>Princípy informačnej bezpečnosti &middot; Fakulta informatiky a informačných technológií</h2>
            <h2>Miroslav Hájek &middot; Server: <?php echo $_SERVER['SERVER_ADDR'] . ':' . $_SERVER['SERVER_PORT']; ?></h2>
            <p>Webová stránka slúžiaca na overenie techník prevencie proti útokom odmietnutia služby
               vyvažovaním záťaže. Aplikácia beží s <b>PHP <?php echo phpversion() . ', ' . apache_get_version(); ?></b>
               Obsahuje galériu obrázkov, ktorá je dostatočne rozsiahla, aby rýchlosť načítanie stránky bola
               merateľná a mala dopad na záťažové testy. Fotky sa vyberajú náhodne z rozsiahlejšieho kalalógu, 
               pri čom si ich návštevník môže kliknutím uložiť do vlastného zoznamu obľubených. Tým testujeme 
               perzistentnosť relácií pri použití load balancera.
            </p> 
        <hr />
            <div class="gallery">
            <?php
                $images = glob('gallery/*.JPG');
                $choices = array_rand($images, 12);
                foreach ($choices as $choice) {
                    echo '<img onclick="bookmark(\'' . $images[$choice] . '\');" src="' . $images[$choice] . '" />';
                }
            ?>
            </div>
        <script type="text/javascript">
            function bookmark(filename) {
                fetch('/bookmark.php', {
                    method: 'POST',
                    headers: {
                        'Content-Type': 'application/x-www-form-urlencoded'
                    },
                    body: 'image=' + filename
                });
            }
        </script>
    </body>
</html>
