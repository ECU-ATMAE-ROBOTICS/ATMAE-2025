import colordetect
import cv2

ball_count = 0


def internal_sort(frame, sep_color):
        """
        Detect color of ball in the frame and return servo command.

        Args:
            frame (np.ndarray): Camera frame (BGR)

        Returns:
            str | None: Serial instruction for Arduino, or #TODO alternate jiggle/shake mode to dislodge stuck balls
        """

        color = colordetect.detect_color(frame)
        colors = {"blue" : 1,
                  "red" : 2,
                  "green" : 3,
                  "yellow" : 4
                    }

        if color is not None and ball_count != 12:
            ball_count += 1
            return colors[color]
        elif ball_count != 12:
            return 5



