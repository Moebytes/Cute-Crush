import React, {useRef} from "react"
import withJuceSlider, {WithJUCESliderProps} from "./withJuceSlider"
import "./styles/buttonslider.scss"

interface Props {
    parameterID: string
    label: string
    style?: React.CSSProperties
}

const ButtonSlider: React.FunctionComponent<Props & WithJUCESliderProps> = ({label, parameterID, 
    style, value, properties, onChange, reset, dragStart, dragEnd}) => {
    const dragRef = useRef({startX: 0, startY: 0, startValue: value})

    const min = properties.start
    const max = properties.end
    const step = (max - min) / (properties.numSteps - 1)

    const normalizeValue = (value: number) => Math.log(value / min) / Math.log(max / min)

    const denormalizeValue = (normalized: number) => min * Math.pow(max / min, normalized)

    const handlePointerDown = (event: React.PointerEvent<HTMLDivElement>) => {
        event.preventDefault()

        dragRef.current = {startX: event.clientX, startY: event.clientY, startValue: value}

        dragStart()
        event.currentTarget.setPointerCapture(event.pointerId)

        const handlePointerMove = (event: PointerEvent) => {
            const dx = event.clientX - dragRef.current.startX
            const dy = dragRef.current.startY - event.clientY

            const sensitivity = event.shiftKey ? 20000 : 2000
            const delta = (dx + dy) / sensitivity

            const startNorm = normalizeValue(dragRef.current.startValue)

            let normalized = startNorm + delta
            normalized = Math.max(0, Math.min(1, normalized))

            let newValue = denormalizeValue(normalized)
            newValue = Math.round((newValue - min) / step) * step + min
            newValue = Math.max(min, Math.min(max, newValue))

            onChange(newValue)
        }

        const handlePointerUp = () => {
            dragEnd()
            window.removeEventListener("pointermove", handlePointerMove)
            window.removeEventListener("pointerup", handlePointerUp)
        }

        window.addEventListener("pointermove", handlePointerMove)
        window.addEventListener("pointerup", handlePointerUp)
    }

    return (
        <div className="button-slider-container" style={{...style}}>
            <div className="button-slider-label">
                {label}
            </div>
            <div className="button-slider" onPointerDown={handlePointerDown}>
                <div className="button-slider-progress">
                    <div className="button-slider-rect" 
                        style={{width: `${normalizeValue(value) * 100}%`}}/>
                </div>
                <span className="button-slider-value">
                    {Math.round(value)} Hz
                </span>
            </div>
        </div>
    )
}

export default withJuceSlider(ButtonSlider)